#include <chrono>

#include <QNetworkRequest>
#include <QByteArray>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

#include "HttpClient.h"

using namespace std;

namespace QTournament
{
  QNetworkAccessManager&getNetworkAccessManager()
  {
    static QNetworkAccessManager instance;
    return instance;
  }

  //----------------------------------------------------------------------------

  QString HttpResponse::getHeader(const QString& hdrName) const
  {
    auto it = headers.find(hdrName);

    return (it == headers.end()) ? "" : it.value();
  }

  //----------------------------------------------------------------------------

  HttpResponse QTournament::HttpClient::blockingRequest(const QString& url, QMap<QString, QString> extraHeaders, const QString& postData, int timeout_ms)
  {
    QNetworkAccessManager& nam = getNetworkAccessManager();

    QNetworkRequest req;
    req.setUrl(QUrl(url));
    for (auto& hdr : extraHeaders.toStdMap())
    {
      req.setRawHeader(hdr.first.toUtf8(), hdr.second.toUtf8());
    }

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(&nam, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    auto start = chrono::high_resolution_clock::now();

    QNetworkReply* re;
    if (postData.isEmpty())
    {
      re = nam.get(req);
    } else {
      re = nam.post(req, postData.toUtf8());
    }
    timer.start(timeout_ms);

    loop.exec();

    // at this point, either the reply is complete or the
    // timer has elapsed


    HttpResponse result;
    auto dt = chrono::high_resolution_clock::now() - start;
    result.roundTripTime_ms = chrono::duration_cast<chrono::milliseconds>(dt).count();

    if (timer.isActive())
    {
      // no timeout, but a completed request where the nam
      // triggered the finished() signal
      timer.stop();
      result.err = (re->error() > 0) ? HttpRequestErr::NetworkError : HttpRequestErr::Okay;
    } else {
      // timeout
      result.err = HttpRequestErr::Timeout;
      re->abort();
    }

    // in case of any errors, we delete the reply object
    // and return and empty HttpResponse
    if (result.err != HttpRequestErr::Okay)
    {
      result.respCode = -1;
      result.roundTripTime_ms = -1;
      re->deleteLater();

      return result;
    }

    // copy all relevant data from the reply object
    result.respCode = re->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.data = re->readAll();
    for (const QByteArray& qba : re->rawHeaderList())
    {
      QString hdrName = QString::fromUtf8(qba);
      QString hdrVal = QString::fromUtf8(re->rawHeader(qba));
      result.headers[hdrName] = hdrVal;
    }

    // delete the object
    re->deleteLater();

    return result;
  }

  HttpResponse HttpClient::blockingRequest(const QString& url, QMap<QString, QString> extraHeaders, const string& postData, int timeout_ms)
  {
    return blockingRequest(url, extraHeaders, QString::fromUtf8(postData.c_str()), timeout_ms);
  }


}

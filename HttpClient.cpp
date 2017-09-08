#include <chrono>

#include <QNetworkRequest>
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

  HttpResponse QTournament::HttpClient::blockingRequest(const QString& url, const QString& postData, int timeout_ms)
  {
    QNetworkAccessManager& nam = getNetworkAccessManager();

    QNetworkRequest req;
    req.setUrl(QUrl(url));

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(&nam, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    auto start = chrono::high_resolution_clock::now();

    QNetworkReply* re;
    re = nam.get(req);
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

    result.respCode = re->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.data = re->readAll();
    re->deleteLater();

    return result;
  }

}

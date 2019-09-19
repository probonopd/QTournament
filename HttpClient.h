#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

#include <QObject>
#include <QNetworkAccessManager>
#include <QMap>

namespace QTournament
{
  enum class HttpRequestErr
  {
    Timeout,
    NetworkError,
    Okay
  };

  //----------------------------------------------------------------------------

  struct HttpResponse
  {
    HttpRequestErr err;

    int respCode;
    QByteArray data;
    int roundTripTime_ms;

    QMap<QString, QString> headers;

    QString getHeader(const QString& hdrName) const;
  };

  //----------------------------------------------------------------------------

  // a "singleton" that returns always the same
  // network access manager instance
  QNetworkAccessManager& getNetworkAccessManager();

  //----------------------------------------------------------------------------

  class HttpClient : public QObject
  {
    Q_OBJECT

  public:
    HttpClient() {}

    HttpResponse blockingRequest(const QString& url,
                                 QMap<QString, QString> extraHeaders = {},
                                 const QString& postData="", int timeout_ms=5000);
    HttpResponse blockingRequest(const QString& url,
                                 QMap<QString, QString> extraHeaders = {},
                                 const std::string& postData="", int timeout_ms=5000);
  };
}
#endif // HTTPCLIENT_H

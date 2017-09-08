#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>

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

    HttpResponse blockingRequest(const QString& url, const QString& postData="", int timeout_ms=5000);
  };
}
#endif // HTTPCLIENT_H

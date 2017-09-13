#ifndef ONLINEMNGR_H
#define ONLINEMNGR_H

#include <memory>

#include <Sloppy/Crypto/Sodium.h>
#include <QObject>
#include <QDate>

using namespace std;

// forward
namespace SqliteOverlay
{
  class KeyValueTab;
}


namespace QTournament
{
  // forward
  class TournamentDB;

  // Erros specific to the online manager
  enum class OnlineError
  {
    // database and key handling
    KeystoreLocked,
    WrongPassword,
    InvalidPassword,
    PasswordAlreadySet,
    DatabaseError,
    KeystoreEmpty,
    InvalidServerSignature,

    // transport layer
    Timeout,
    BadRequest,
    BadResponse,      // e.g, no nonce in response

    // (partial) success
    TransportOkay_AppError,
    Okay
  };

  // online registration data as a struct
  struct OnlineRegistrationData
  {
    QString tnmtName;
    QString club;
    QString personName;
    QString email;
    QDate firstDay;
    QDate lastDay;
  };

  // some type simplifications
  using PubSignKey = Sloppy::Crypto::SodiumLib::AsymSign_PublicKey;
  using SecSignKey = Sloppy::Crypto::SodiumLib::AsymSign_SecretKey;
  using SecretBox = Sloppy::Crypto::PasswordProtectedSecret;

  class OnlineMngr
  {
  public:
    static constexpr int NonceLength = 10;
    static constexpr const char* ServerPubKey_B64 = "gxgUevEXPrlHKluvDFUVOVZqf9dhR0+Ae3OrmRhYM1o=";

    OnlineMngr(TournamentDB* _db, const QString& _apiBaseUrl, int _defaultTimeout_ms);

    // transport layer
    OnlineError execSignedServerRequest(const QString& subUrl, const QByteArray& postData, QByteArray& responseOut);

    // password / keybox management
    bool hasSecretInDatabase() const;
    OnlineError setPassword(const QString& newPassword, const QString& oldPassword="");
    OnlineError isCorrectPassword(const QString& pw) const;
    OnlineError unlockKeystore(const QString& pw);
    bool isUnlocked() const { return secKeyUnlocked; }

    // server requests
    int ping();
    OnlineError registerTournament(const OnlineRegistrationData& ord, QString& errCodeOut);

  protected:
    bool initKeyboxWithFreshKeys(const QString& pw);

  private:
    TournamentDB* db;
    QString apiBaseUrl;
    int defaultTimeout_ms;
    Sloppy::Crypto::SodiumLib* cryptoLib;
    unique_ptr<SqliteOverlay::KeyValueTab> cfgTab;
    SecSignKey secKey;
    PubSignKey pubKey;
    bool secKeyUnlocked;
    PubSignKey srvPubKey;

  };

}
#endif // ONLINEMNGR_H

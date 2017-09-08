#ifndef ONLINEMNGR_H
#define ONLINEMNGR_H

#include <memory>

#include <Sloppy/Crypto/Sodium.h>
#include <QObject>

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
    KeystoreLocked,
    WrongPassword,
    InvalidPassword,
    PasswordAlreadySet,
    DatabaseError,
    KeystoreEmpty,
    Okay
  };

  // some type simplifications
  using PubSignKey = Sloppy::Crypto::SodiumLib::AsymSign_PublicKey;
  using SecSignKey = Sloppy::Crypto::SodiumLib::AsymSign_SecretKey;
  using SecretBox = Sloppy::Crypto::PasswordProtectedSecret;

  class OnlineMngr
  {
  public:
    OnlineMngr(TournamentDB* _db, const QString& _apiBaseUrl, int _defaultTimeout_ms);

    // password / keybox management
    bool hasSecretInDatabase() const;
    OnlineError setPassword(const QString& newPassword, const QString& oldPassword="");
    OnlineError isCorrectPassword(const QString& pw) const;
    OnlineError unlockKeystore(const QString& pw);

    // server requests
    int ping();

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
  };

}
#endif // ONLINEMNGR_H

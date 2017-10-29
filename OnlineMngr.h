#ifndef ONLINEMNGR_H
#define ONLINEMNGR_H

#include <memory>

#include <Sloppy/Crypto/Sodium.h>
#include <Sloppy/DateTime/DateAndTime.h>

#include <QObject>
#include <QDate>

using namespace std;

// forward
namespace SqliteOverlay
{
  class KeyValueTab;
  class ChangeLogEntry;
}


namespace QTournament
{
  // forward
  class TournamentDB;

  //----------------------------------------------------------------------------

  // Erros specific to the online manager
  enum class OnlineError
  {
    // database and key handling
    KeystoreLocked,
    WrongPassword,
    InvalidPassword,
    PasswordAlreadySet,
    LocalDatabaseError,
    KeystoreEmpty,
    InvalidServerSignature,
    NoSession,

    // transport layer
    Timeout,
    BadRequest,
    BadResponse,      // e.g, no nonce in response

    // compatibility
    IncompatibleVersions,

    // (partial) success
    TransportOkay_AppError,
    Okay
  };

  //----------------------------------------------------------------------------

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

  //----------------------------------------------------------------------------

  // all data related to syncs
  struct SyncState
  {
    string sessionKey;
    Sloppy::DateTime::UTCTimestamp connStart;
    Sloppy::DateTime::UTCTimestamp lastFullSync;
    Sloppy::DateTime::UTCTimestamp lastPartialSync;
    int partialSyncCounter;

    size_t lastDbChangelogLen;
    Sloppy::DateTime::UTCTimestamp lastChangelogLenCheck;

    SyncState()
      :sessionKey{},
       connStart{1900,1,1,0,0,0},  // 1900-01-01 as a dummy value for "not set"
       lastFullSync{1900,1,1,0,0,0},
       lastPartialSync{1900,1,1,0,0,0},
       partialSyncCounter{-1},
       lastDbChangelogLen{0},
       lastChangelogLenCheck{1900,1,1,0,0,0} {}

    bool hasSession() const { return (!(sessionKey.empty())); }
  };

  //----------------------------------------------------------------------------

  // some type simplifications
  using PubSignKey = Sloppy::Crypto::SodiumLib::AsymSign_PublicKey;
  using SecSignKey = Sloppy::Crypto::SodiumLib::AsymSign_SecretKey;
  using SecretBox = Sloppy::Crypto::PasswordProtectedSecret;

  //----------------------------------------------------------------------------

  static constexpr const char* ImplementedProtoVersion = "1.0";

  //----------------------------------------------------------------------------
  class OnlineMngr
  {
  public:
    static constexpr int NonceLength = 10;
    static constexpr const char* ServerPubKey_B64 = "gxgUevEXPrlHKluvDFUVOVZqf9dhR0+Ae3OrmRhYM1o=";
    static constexpr int DatabaseInactiveBeforeSync_secs = 5;
    static constexpr const char* DefaultApiBaseUrl = "http://localhost:7777/api/tournament";
    static constexpr int DefaultServerTimeout_ms = 7000;

    // the following to consts would belong into TournamentDataDefs.h, but
    // I don't want to recompile everthing for these three strings
    static constexpr const char* CfgKey_CustomServer = "CustomServerBaseUrl";
    static constexpr const char* CfgKey_CustomServerKey = "CustomServerKey";
    static constexpr const char* CfgKey_CustomServerTimeout = "CustomServerTimeout";

    OnlineMngr(TournamentDB* _db);

    // transport layer
    OnlineError execSignedServerRequest(const QString& subUrl, bool withSession, const QByteArray& postData, QByteArray& responseOut);

    // password / keybox management
    bool hasSecretInDatabase() const;
    OnlineError setPassword(const QString& newPassword, const QString& oldPassword="");
    OnlineError isCorrectPassword(const QString& pw) const;
    OnlineError unlockKeystore(const QString& pw);
    bool isUnlocked() const { return secKeyUnlocked; }
    bool hasRegistrationSubmitted() const;

    // server requests
    int ping();
    OnlineError registerTournament(const OnlineRegistrationData& ord, QString& errCodeOut);
    OnlineError startSession(QString& errCodeOut);
    bool disconnect();
    OnlineError deleteFromServer(QString& errCodeOut);

    // sync
    OnlineError doFullSync(QString& errCodeOut);
    bool wantsToSync();
    OnlineError doPartialSync(QString& errCodeOut);

    // status info for the GUI
    SyncState getSyncState() const;

    int getLastReqTime_ms() const { return lastReqTime_ms; }

  protected:
    bool initKeyboxWithFreshKeys(const QString& pw);
    void compactDatabaseChangeLog(vector<SqliteOverlay::ChangeLogEntry>& log);
    string log2SyncString(const vector<SqliteOverlay::ChangeLogEntry>& log);

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
    SyncState syncState;
    int lastReqTime_ms;
  };

}
#endif // ONLINEMNGR_H

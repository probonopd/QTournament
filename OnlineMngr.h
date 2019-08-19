#ifndef ONLINEMNGR_H
#define ONLINEMNGR_H

#include <memory>

#include <Sloppy/Crypto/Sodium.h>
#include <Sloppy/DateTime/DateAndTime.h>

#include <QObject>
#include <QDate>

#include <SqliteOverlay/KeyValueTab.h>

// forward
namespace SqliteOverlay
{
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
    LocalDatabaseBusy,

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
    std::string sessionKey;
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
#ifdef RELEASE_BUILD
    static constexpr const char* DefaultApiBaseUrl = "http://qtournament.de/api";  // production
    static constexpr const char* ServerPubKey_B64 = "ACsAKgpqvQuxIFUsDqzI5KD2DrGxyTVPgDSIFrX4CHI=";  // production
#else
    static constexpr const char* DefaultApiBaseUrl = "http://localhost:7777/api/tournament";          // local test
    static constexpr const char* ServerPubKey_B64 = "gxgUevEXPrlHKluvDFUVOVZqf9dhR0+Ae3OrmRhYM1o=";   // local test
#endif
    static constexpr int DatabaseInactiveBeforeSync_secs = 5;
    static constexpr int DefaultServerTimeout_ms = 7000;

    // the following to consts would belong into TournamentDataDefs.h, but
    // I don't want to recompile everthing for these three strings
    static constexpr const char* CfgKey_CustomServer = "CustomServerBaseUrl";
    static constexpr const char* CfgKey_CustomServerKey = "CustomServerKey";
    static constexpr const char* CfgKey_CustomServerTimeout = "CustomServerTimeout";

    OnlineMngr(QTournament::TournamentDB& _db);

    // transport layer
    OnlineError execSignedServerRequest(const QString& subUrl, bool withSession, const QByteArray& postData, QByteArray& responseOut);

    // password / keybox management
    bool hasSecretInDatabase();
    OnlineError setPassword(const QString& newPassword, const QString& oldPassword="");
    OnlineError isCorrectPassword(const QString& pw);
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

    // custom connection settings
    QString getCustomUrl();
    bool setCustomUrl(const QString& url);
    QString getCustomServerKey();
    bool setCustomServerKey(const QString& key);
    int getCustomTimeout_ms();
    bool setCustomTimeout_ms(int newTimeout);
    void applyCustomServerSettings();

  protected:
    bool initKeyboxWithFreshKeys(const QString& pw);
    void compactDatabaseChangeLog(std::vector<SqliteOverlay::ChangeLogEntry>& log);
    std::string log2SyncString(const SqliteOverlay::ChangeLogList& log);

  private:
    std::reference_wrapper<QTournament::TournamentDB> db;
    QString apiBaseUrl;
    int defaultTimeout_ms;
    Sloppy::Crypto::SodiumLib* cryptoLib;
    SqliteOverlay::KeyValueTab cfgTab;
    SecSignKey secKey;
    PubSignKey pubKey;
    bool secKeyUnlocked;
    PubSignKey srvPubKey;
    SyncState syncState;
    int lastReqTime_ms;
  };

}
#endif // ONLINEMNGR_H

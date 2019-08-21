#include <iostream>
#include <chrono>

#include <Sloppy/json.hpp>
#include <Sloppy/Crypto/Crypto.h>
#include <Sloppy/Crypto/Sodium.h>
#include <SqliteOverlay/KeyValueTab.h>

#include "TournamentDB.h"
#include "OnlineMngr.h"
#include "HttpClient.h"
#include "TeamMngr.h"
#include "CourtMngr.h"
#include "CatMngr.h"
#include "MatchMngr.h"
#include "PlayerMngr.h"
#include "RankingMngr.h"
#include "HelperFunc.h"

using namespace std;

namespace QTournament
{

  OnlineMngr::OnlineMngr(TournamentDB& _db)
    :db{_db}, cryptoLib{Sloppy::Crypto::SodiumLib::getInstance()},
      cfgTab{SqliteOverlay::KeyValueTab{db, TabCfg}}, secKeyUnlocked{false},
      syncState{}, lastReqTime_ms{-1}
  {
    applyCustomServerSettings();
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::execSignedServerRequest(const QString& subUrl, bool withSession, const QByteArray& postData, QByteArray& responseOut)
  {
    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // every request will be preceeded by a 10-character
    // random nonce to avoid replay attacks
    //
    // for sessioned requests, we also insert the session key
    if (withSession && syncState.sessionKey.empty()) return OnlineError::NoSession;
    string nonce = Sloppy::Crypto::getRandomAlphanumString(NonceLength);
    string body{nonce};
    if (withSession) body += syncState.sessionKey;
    body += string{postData.constData()};

    // create a detached signature of the body
    auto sig = cryptoLib->sign_detached(body, secKey);
    string sigB64 = sig.toBase64();

    // put the signature in an extra header
    QMap<QString, QString> hdr;
    hdr["X-Signature"] = QString::fromUtf8(sigB64.c_str());

    // add version information
    hdr["X-ProtocolVersion"] = QString::fromUtf8(ImplementedProtoVersion);
    auto _dv = cfgTab.getString2(CfgKey_DbVersion);
    string dv = _dv.value_or("unknown");
    QString dbVersion = QString::fromUtf8(dv.c_str());
    hdr["X-DatabaseVersion"] = dbVersion;

    // send the request
    HttpClient cli;
    QString url = apiBaseUrl + subUrl;
    auto startTime = chrono::high_resolution_clock::now();
    HttpResponse re = cli.blockingRequest(url, hdr, body, defaultTimeout_ms);
    auto _elapsedTime = chrono::high_resolution_clock::now() - startTime;
    lastReqTime_ms = chrono::duration_cast<chrono::milliseconds>(_elapsedTime).count();

    // did we get a response?
    if (re.respCode < 0) return OnlineError::Timeout;
    if (re.respCode != 200) return OnlineError::BadRequest;

    // check for a plain "INCOMPATIBLE" message without signature
    if (re.data.startsWith(QByteArray::fromStdString("INCOMPATIBLE")))
    {
      return OnlineError::IncompatibleVersions;
    }

    // we're compatible, so check the responses signature
    sigB64 = string{re.getHeader("X-Signature").toUtf8().constData()};
    if (sigB64.empty()) return OnlineError::InvalidServerSignature;
    sig.fillFromBase64(sigB64);
    Sloppy::MemView respDataView{re.data.constData(), static_cast<size_t>(re.data.size())};
    bool isOkay = cryptoLib->sign_verify_detached(respDataView, sig, srvPubKey);
    if (!isOkay) return OnlineError::InvalidServerSignature;

    // the response should be preceeded by a 10-character nonce
    if (re.data.size() < NonceLength) return OnlineError::BadResponse;

    // the response-nonce has to be indentical with the request nonce,
    // otherwise this could be a replay
    if (!(re.data.startsWith(QByteArray::fromStdString(nonce))))
    {
      return OnlineError::BadResponse;
    }

    // the signature and nonce are okay so we can trust the response

    responseOut = re.data.right(re.data.size() - NonceLength);
    return OnlineError::Okay;
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::hasSecretInDatabase()
  {
    if (!(cfgTab.hasKey(CfgKey_Keystore))) return false;

    const string encData = cfgTab[CfgKey_Keystore];
    return (!(encData.empty()));
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::setPassword(const QString& newPassword, const QString& oldPassword)
  {
    if (newPassword.isEmpty()) return OnlineError::InvalidPassword;

    bool hasExistingPassword = hasSecretInDatabase();

    // check password update conditions
    if (hasExistingPassword && oldPassword.isEmpty()) return OnlineError::PasswordAlreadySet;

    // initialization of an empty keystore
    if (!hasExistingPassword)
    {
      bool isOkay = initKeyboxWithFreshKeys(newPassword);
      return isOkay ? OnlineError::Okay : OnlineError::LocalDatabaseError;
    }

    //
    // password update process
    //

    // initialize a PasswordProtectedSecret from the
    // encrypted data and try the provided password
    const string encData = cfgTab[CfgKey_Keystore];
    SecretBox box(encData, true);
    bool isOkay = box.setPassword(oldPassword.toUtf8().constData());
    if (!isOkay) return OnlineError::InvalidPassword;

    // call the password updater
    isOkay = box.changePassword(oldPassword.toUtf8().constData(), newPassword.toUtf8().constData());
    if (!isOkay) return OnlineError::InvalidPassword;  // shouldn't happen

    // store the new encrypted data in the database
    string cipher = box.asString(true);
    if (cipher.empty()) return OnlineError::InvalidPassword;
    cfgTab.set(CfgKey_Keystore, cipher);
    const string& readBack = cfgTab[CfgKey_Keystore];
    if (readBack != cipher) return OnlineError::LocalDatabaseError;

    return OnlineError::Okay;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::isCorrectPassword(const QString& pw)
  {
    if (!(hasSecretInDatabase())) return OnlineError::KeystoreEmpty;

    // initialize a PasswordProtectedSecret from the
    // encrypted data and use the built-in password checker
    const string encData = cfgTab[CfgKey_Keystore];
    SecretBox box(encData, true);
    bool isValid = box.isValidPassword(pw.toUtf8().constData());

    return isValid ? OnlineError::Okay : OnlineError::InvalidPassword;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::unlockKeystore(const QString& pw)
  {
    if (!(hasSecretInDatabase())) return OnlineError::KeystoreEmpty;

    if (pw.isEmpty()) return OnlineError::InvalidPassword;

    // initialize a PasswordProtectedSecret from the
    // encrypted data
    const string encData = cfgTab[CfgKey_Keystore];
    SecretBox box(encData, true);
    bool isOkay = box.setPassword(pw.toUtf8().constData());
    if (!isOkay) return OnlineError::WrongPassword;

    // the box is unlocked. Now copy the secret signing key
    // over to our local key
    string secKeyAsString = box.getSecretAsString();
    if (secKeyAsString.empty()) return OnlineError::LocalDatabaseError;
    secKey.fillFromString(secKeyAsString);
    secKeyUnlocked = true;

    // extract the public key from the secret key
    isOkay = cryptoLib->genPublicSignKeyFromSecretKey(secKey, pubKey);
    return isOkay ? OnlineError::Okay : OnlineError::LocalDatabaseError;
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::hasRegistrationSubmitted() const
  {
    return cfgTab.hasKey(CfgKey_RegistrationTimestamp);
  }

  //----------------------------------------------------------------------------

  int OnlineMngr::ping()
  {
    QString url = apiBaseUrl + "/ping";

    HttpClient cli;
    HttpResponse re = cli.blockingRequest(url, {}, string{}, defaultTimeout_ms);

    return re.roundTripTime_ms;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::registerTournament(const OnlineRegistrationData& ord, QString& errCodeOut)
  {
    errCodeOut.clear();

    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // convert the registration data to JSON
    //
    // no error checking here, we leave this to the server
    auto dic = nlohmann::json::object();
    dic["name"] = ord.tnmtName.toUtf8().constData();
    dic["club"] = ord.club.toUtf8().constData();
    dic["owner"] = ord.personName.toUtf8().constData();
    dic["email"] = ord.email.toUtf8().constData();
    dic["first"] = ord.firstDay.year() * 10000 + ord.firstDay.month() * 100 + ord.firstDay.day();
    dic["last"] = ord.lastDay.year() * 10000 + ord.lastDay.month() * 100 + ord.lastDay.day();
    dic["pubkey"] = pubKey.toBase64();

    // do the actual server request
    QByteArray response;
    OnlineError err = execSignedServerRequest("/registration", false, QByteArray{dic.dump().c_str()}, response);
    if (err != OnlineError::Okay) return err;

    // construct the reply
    errCodeOut.clear();

    errCodeOut = QString::fromUtf8(response.constData());
    OnlineError result = (errCodeOut == "OK") ? OnlineError::Okay : OnlineError::TransportOkay_AppError;

    // if successful, store the registration timestamp in the database
    if (result == OnlineError::Okay)
    {
      UTCTimestamp now;
      cfgTab.set(CfgKey_RegistrationTimestamp, now);
    }

    return result;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::startSession(QString& errCodeOut)
  {
    errCodeOut.clear();

    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // do the actual server request
    string pkB64 = pubKey.toBase64();
    QByteArray response;
    OnlineError err = execSignedServerRequest("/startSession", false, QByteArray(pkB64.c_str()), response);
    if (err != OnlineError::Okay) return err;

    // extract the session key, if we were successful
    errCodeOut = QString::fromUtf8(response.constData());
    if (errCodeOut.startsWith("OK"))
    {
      syncState.sessionKey = string{errCodeOut.mid(2).toUtf8().constData()};
      syncState.connStart = UTCTimestamp();
    } else {
      return OnlineError::TransportOkay_AppError;
    }

    // force a full sync at session start
    err = doFullSync(errCodeOut);

    // if the sync was successfull,
    // enable the database changelog,
    // otherwise terminate the session
    if (err != OnlineError::Okay) return err;

    if (errCodeOut != "OK")
    {
      disconnect();
      return OnlineError::TransportOkay_AppError;
    }

    db.get().enableChangeLog(true);
    return OnlineError::Okay;
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::disconnect()
  {
    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return false;
    }

    // we need an active server session
    if (!(syncState.hasSession()))
    {
      return false;
    }

    QByteArray response;
    OnlineError err = execSignedServerRequest("/terminateSession", true, QByteArray{}, response);
    db.get().disableChangeLog(true);
    syncState = SyncState{};  // reset all clocks, session keys, etc.

    cout << "Terminate Session, server said: " << response.constData() << endl;

    string errCodeOut = string{response.constData()};

    return ((err == OnlineError::Okay) && (errCodeOut == "BYE"));
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::deleteFromServer(QString& errCodeOut)
  {
    errCodeOut.clear();

    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // disconnect any running sessions;
    // it's harmless to call this if we don't
    // have a session
    disconnect();

    // do the actual request
    string pkB64 = pubKey.toBase64();
    QByteArray response;
    OnlineError err = execSignedServerRequest("/deleteTournament", false, QByteArray(pkB64.c_str()), response);
    if (err != OnlineError::Okay) return err;

    // the request was okay on a transport / crypto level.
    // did we also succeed on application level?
    errCodeOut = QString::fromUtf8(response.constData());
    if (errCodeOut != "OK") return OnlineError::TransportOkay_AppError;

    try
    {
      // prepare to delete everything server-related from the database
      auto trans = db.get().startTransaction();

      for (const string& keyName : {CfgKey_Keystore, CfgKey_RegistrationTimestamp,
                                    CfgKey_CustomServer, CfgKey_CustomServerKey, CfgKey_CustomServerTimeout})
      {
        cfgTab.remove(keyName);
      }

      // commit all changes at once
      trans.commit();
    }
    catch (SqliteOverlay::BusyException&)
    {
      return OnlineError::LocalDatabaseError;
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return OnlineError::LocalDatabaseError;
    }

    // delete all internal state
    secKey = SecSignKey{};
    pubKey = PubSignKey{};
    secKeyUnlocked = false;
    syncState = SyncState{};
    lastReqTime_ms = -1;

    return OnlineError::Okay;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::doFullSync(QString& errCodeOut)
  {
    errCodeOut.clear();

    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // we need an active server session
    if (!(syncState.hasSession()))
    {
      return OnlineError::NoSession;
    }

    //
    // collect all CSV-data
    //
    string csv;

    // courts
    CourtMngr cm{db};
    csv += cm.getSyncString({});

    // Teams
    TeamMngr tm{db};
    csv += tm.getSyncString({});

    // players
    PlayerMngr pm{db};
    csv += pm.getSyncString({});
    csv += pm.getSyncString_P2C({});
    csv += pm.getSyncString_Pairs({});

    // categories
    CatMngr caMngr{db};
    csv += caMngr.getSyncString({});

    // matches
    MatchMngr mm{db};
    csv += mm.getSyncString({});
    csv += mm.getSyncString_MatchGroups({});

    // rankings
    RankingMngr rm{db};
    csv += rm.getSyncString({});

    cout << csv << endl;

    cout << endl << "Total size: " << csv.size() << endl;

    QByteArray response;
    OnlineError err = execSignedServerRequest("/fullSync", true, QByteArray(csv.c_str()), response);
    if (err != OnlineError::Okay) return err;

    errCodeOut = QString::fromUtf8(response.constData());
    if (errCodeOut == "OK0")
    {
      UTCTimestamp now;
      syncState.lastFullSync = now;
      syncState.lastPartialSync  = now;
      syncState.partialSyncCounter = 0;
      syncState.lastDbChangelogLen = 0;
      syncState.lastChangelogLenCheck = now;
      errCodeOut = "OK";
      return OnlineError::Okay;
    }

    return OnlineError::TransportOkay_AppError;
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::wantsToSync()
  {
    if (!(syncState.hasSession())) return false;

    size_t logLen = db.get().getChangeLogLength();
    if (logLen == 0) return false;

    // check the "inactivity hystersis"
    UTCTimestamp now;
    TimePeriod dt{syncState.lastChangelogLenCheck, now};
    if (dt.getLength_Sec() < DatabaseInactiveBeforeSync_secs) return false;

    // if, after the delay, new changes have occured, reset the delay
    if (logLen > syncState.lastDbChangelogLen)
    {
      syncState.lastDbChangelogLen = logLen;
      syncState.lastChangelogLenCheck = now;
      return false;
    }

    // no more changes within the hystersis period
    // ==> sync!
    return true;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::doPartialSync(QString& errCodeOut)
  {
    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // we need an active server session
    if (!(syncState.hasSession()))
    {
      return OnlineError::NoSession;
    }

    // make sure noone interferes with the database right now
    //
    // technically this shouldn't be necessary because we're
    // basically single-threaded and we don't return to the
    // event loop before we're finished. But better safe than
    // sorry...
    auto trans = db.get().startTransaction(SqliteOverlay::TransactionType::Exclusive);

    // get all recent database changes
    auto log = db.get().getAllChangesAndClearQueue();
    if (log.empty()) return OnlineError::Okay;

    // remove unnecessary, redundant entries from the log
    compactDatabaseChangeLog(log);

    // get the CSV update string
    string csv = log2SyncString(log);

    // trigger the update
    QByteArray response;
    OnlineError err = execSignedServerRequest("/partialSync", true, QByteArray(csv.c_str()), response);
    if (err != OnlineError::Okay) return err;

    errCodeOut = QString::fromUtf8(response.constData());
    if (errCodeOut.startsWith("OK"))
    {
      int serverSyncCount = errCodeOut.mid(2).toInt();
      if (serverSyncCount != (syncState.partialSyncCounter + 1))
      {
        cerr << "Server and Client are out of sync! Connection forcefully closed!" << endl;
        disconnect();
        return OnlineError::TransportOkay_AppError;
      }

      errCodeOut = "OK";

      UTCTimestamp now;
      syncState.lastPartialSync  = now;
      ++syncState.partialSyncCounter;
      syncState.lastDbChangelogLen = 0;
      syncState.lastChangelogLenCheck = now;
      return OnlineError::Okay;
    }

    return OnlineError::TransportOkay_AppError;
  }

  //----------------------------------------------------------------------------

  SyncState OnlineMngr::getSyncState() const
  {
    SyncState result{syncState};

    // do not leak the session key
    if (!(result.sessionKey.empty())) result.sessionKey = "XXX";

    return result;
  }

  //----------------------------------------------------------------------------

  QString OnlineMngr::getCustomUrl()
  {
    if (cfgTab.hasKey(CfgKey_CustomServer))
    {
      return QString::fromUtf8(cfgTab[CfgKey_CustomServer].c_str());
    }

    return "";
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::setCustomUrl(const QString& url)
  {
    try
    {
      if (url.isEmpty())
      {
        cfgTab.remove(CfgKey_CustomServer);
        return true;
      }

      cfgTab.set(CfgKey_CustomServer, QString2StdString(url));

      apiBaseUrl = url;

      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  //----------------------------------------------------------------------------

  QString OnlineMngr::getCustomServerKey()
  {
    return stdString2QString(
          cfgTab.getString2(CfgKey_CustomServerKey).value_or("")
          );
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::setCustomServerKey(const QString& key)
  {
    try
    {
      if (key.isEmpty())
      {
        cfgTab.remove(CfgKey_CustomServerKey);
        return true;
      }

      string sKey = key.toUtf8().constData();
      PubSignKey testKey;
      bool isOkay = testKey.fillFromBase64(sKey);
      if (!isOkay) return false;

      cfgTab.set(CfgKey_CustomServerKey, sKey);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  //----------------------------------------------------------------------------

  int OnlineMngr::getCustomTimeout_ms()
  {
    return cfgTab.getInt2(CfgKey_CustomServerTimeout).value_or(-1);
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::setCustomTimeout_ms(int newTimeout)
  {
    try
    {
      if (newTimeout < 0)
      {
        cfgTab.remove(CfgKey_CustomServerTimeout);
        return true;
      }

      if (newTimeout < 1000) return false;  // invalid range, we want at least one second
      cfgTab.set(CfgKey_CustomServerTimeout, newTimeout);

      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  //----------------------------------------------------------------------------

  void OnlineMngr::applyCustomServerSettings()
  {
    // do we have a custom server or do we use the default?
    auto srv = cfgTab.getString2(CfgKey_CustomServer);
    if (srv.has_value())
    {
      apiBaseUrl = "http://" + stdString2QString(srv.value());
    } else {
      apiBaseUrl = DefaultApiBaseUrl;
    }

    // do we have a custom key or do we use the default?
    //
    // no error checking, we check the values before writing to the DB
    auto pubKey = cfgTab.getString2(CfgKey_CustomServerKey);
    if (pubKey.has_value())
    {
      srvPubKey.fillFromBase64(pubKey.value());
    } else {
      srvPubKey.fillFromBase64(ServerPubKey_B64);
    }

    // do we have a custom timeout or do we use the default?
    auto to = cfgTab.getInt2(CfgKey_CustomServerTimeout);
    defaultTimeout_ms = to.value_or(DefaultServerTimeout_ms);
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::initKeyboxWithFreshKeys(const QString& pw)
  {
    if (pw.isEmpty()) return false;

    // prepare a PasswordProtectedSecret as provided
    // by Sloppy
    SecretBox box;
    bool isOkay = box.setPassword(pw.toUtf8().constData());
    if (!isOkay) return false;

    // create a new pair of keys
    SecSignKey localSecKey;
    PubSignKey localPubKey;
    cryptoLib->genAsymSignKeyPair(localPubKey, localSecKey);

    // store the secret key in the key box
    isOkay = box.setSecret(localSecKey.toMemView());
    if (!isOkay) return false;

    // store the encrypted key box in our database
    string cipher = box.asString(true);
    if (cipher.empty()) return false;
    cfgTab.set(CfgKey_Keystore, cipher);
    const string readBack = cfgTab[CfgKey_Keystore];
    if (readBack != cipher) return false;

    // overwrite the currently used keys
    secKey = SecSignKey{localSecKey};
    pubKey = PubSignKey{localPubKey};
    secKeyUnlocked = true;

    return true;
  }

  //----------------------------------------------------------------------------

  void OnlineMngr::compactDatabaseChangeLog(vector<SqliteOverlay::ChangeLogEntry>& log)
  {
    size_t oldLen = log.size();

    // step one:
    // search from the end of the list if there are any prior
    // updates for the same row. If yes, remove the prior update
    // because we'll always transmit the whole row
    size_t outerIdx = log.size();
    while (outerIdx > 0)
    {
      --outerIdx;

      const SqliteOverlay::ChangeLogEntry cle = log[outerIdx];  // do NOT use references here, because content gets shifted in memory when deleted
      if (cle.action != SqliteOverlay::RowChangeAction::Update) continue;

      size_t innerIdx = outerIdx;
      while (innerIdx != 0)
      {
        --innerIdx;
        const SqliteOverlay::ChangeLogEntry& inner = log.at(innerIdx);
        if ((inner.rowId == cle.rowId) &&
            (inner.action == SqliteOverlay::RowChangeAction::Update) &&
            (inner.tabName == cle.tabName))
        {
          log.erase(log.begin() + innerIdx);

          // now innerIdx points to the element after the deleted
          // element but since we're doing a "--" in the loop's head
          // that's fine

          // but we have to adjust the value of the outerIdx!
          --outerIdx;
        }
      }
    }

    // step two:
    // if there is an deletion, remove prior insertions or updates of the same row
    outerIdx = log.size();
    while (outerIdx > 0)
    {
      --outerIdx;

      const SqliteOverlay::ChangeLogEntry cle = log[outerIdx];  // do NOT use references here, because content gets shifted in memory when deleted
      if (cle.action != SqliteOverlay::RowChangeAction::Delete) continue;

      bool foundInsert = false;
      size_t innerIdx = outerIdx;
      while (innerIdx != 0)
      {
        --innerIdx;
        const SqliteOverlay::ChangeLogEntry& inner = log.at(innerIdx);
        if ((inner.rowId == cle.rowId) && (inner.tabName == cle.tabName))
        {
          foundInsert = (inner.action == SqliteOverlay::RowChangeAction::Insert);

          log.erase(log.begin() + innerIdx);

          // now innerIdx points to the element after the deleted
          // element but since we're doing a "--" in the loop's head
          // that's fine

          // but we have to adjust the value of the outerIdx!
          --outerIdx;

          // there is no need to go back before the first insert
          if (foundInsert) break;
        }
      }

      // if we found and deleted the insert, we can also delete
      // the deletion
      if (foundInsert) log.erase(log.begin() + outerIdx);
    }

    cerr << "Log compacter could delete " << (oldLen - log.size()) << " entries!" << endl;
  }

  //----------------------------------------------------------------------------

  string OnlineMngr::log2SyncString(const SqliteOverlay::ChangeLogList& log)
  {
    // copy the log
    SqliteOverlay::ChangeLogList cll = log;

    // sort copied entries by table name
    std::stable_sort(cll.begin(), cll.end(), [](const SqliteOverlay::ChangeLogEntry& e1, const SqliteOverlay::ChangeLogEntry& e2)
    {
      return (e1.tabName < e2.tabName);
    });

    // append a dummy entry at the end that triggers
    // a bogus tablename change in the following algorithm.
    // the dummy entry never makes it to the result string
    cll.push_back(SqliteOverlay::ChangeLogEntry{SqliteOverlay::RowChangeAction::Delete, "xxx", "___", 42});

    string result;
    string curTabName;
    std::vector<int> idxList;
    for (auto it = cll.begin(); it != cll.end(); ++it)
    {
      const auto& cle = *it;

      if (cle.tabName != curTabName)
      {
        if (!(idxList.empty()))
        {
          if (curTabName == TabCourt)
          {
            CourtMngr mngr{db};
            result += mngr.getSyncString(idxList);
          }
          if (curTabName == TabTeam)
          {
            TeamMngr mngr{db};
            result += mngr.getSyncString(idxList);
          }
          if (curTabName == TabPlayer)
          {
            PlayerMngr mngr{db};
            result += mngr.getSyncString(idxList);
          }
          if (curTabName == TabP2C)
          {
            PlayerMngr mngr{db};
            result += mngr.getSyncString_P2C(idxList);
          }
          if (curTabName == TabPairs)
          {
            PlayerMngr mngr{db};
            result += mngr.getSyncString_Pairs(idxList);
          }
          if (curTabName == TabCategory)
          {
            CatMngr mngr{db};
            result += mngr.getSyncString(idxList);
          }
          if (curTabName == TabMatch)
          {
            MatchMngr mngr{db};
            result += mngr.getSyncString(idxList);
          }
          if (curTabName == TabMatchGroup)
          {
            MatchMngr mngr{db};
            result += mngr.getSyncString_MatchGroups(idxList);
          }
          if (curTabName == TabMatchSystem)
          {
            RankingMngr mngr{db};
            result += mngr.getSyncString(idxList);
          }
        }

        curTabName = cle.tabName;
        idxList.clear();
      }

      if (cle.action == SqliteOverlay::RowChangeAction::Delete)
      {
        idxList.push_back(- cle.rowId);  // negative ID ==> deletion
      } else {
        idxList.push_back(cle.rowId);
      }
    }

    return result;
  }

  //----------------------------------------------------------------------------

}

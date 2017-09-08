#include <SqliteOverlay/KeyValueTab.h>

#include "TournamentDB.h"
#include "OnlineMngr.h"
#include "HttpClient.h"

namespace QTournament
{

  OnlineMngr::OnlineMngr(TournamentDB* _db, const QString& _apiBaseUrl, int _defaultTimeout_ms)
    :db{_db}, apiBaseUrl{_apiBaseUrl}, defaultTimeout_ms{_defaultTimeout_ms},
     cryptoLib{Sloppy::Crypto::SodiumLib::getInstance()},
     cfgTab{SqliteOverlay::KeyValueTab::getTab(db, TAB_CFG)}, secKeyUnlocked{false}
  {
  }

  //----------------------------------------------------------------------------

  bool OnlineMngr::hasSecretInDatabase() const
  {
    if (!(cfgTab->hasKey(CFG_KEY_KEYSTORE))) return false;

    const string& encData = cfgTab->operator [](CFG_KEY_KEYSTORE);
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
      return isOkay ? OnlineError::Okay : OnlineError::DatabaseError;
    }

    //
    // password update process
    //

    // initialize a PasswordProtectedSecret from the
    // encrypted data and try the provided password
    const string& encData = cfgTab->operator [](CFG_KEY_KEYSTORE);
    SecretBox box(encData, true);
    bool isOkay = box.setPassword(oldPassword.toUtf8().constData());
    if (!isOkay) return OnlineError::InvalidPassword;

    // call the password updater
    isOkay = box.changePassword(oldPassword.toUtf8().constData(), newPassword.toUtf8().constData());
    if (!isOkay) return OnlineError::InvalidPassword;  // shouldn't happen

    // store the new encrypted data in the database
    string cipher = box.asString(true);
    if (cipher.empty()) return OnlineError::InvalidPassword;
    int err;
    cfgTab->set(CFG_KEY_KEYSTORE, cipher, &err);
    if (err != SQLITE_DONE) return OnlineError::DatabaseError;
    const string& readBack = cfgTab->operator [](CFG_KEY_KEYSTORE);
    if (readBack != cipher) return OnlineError::DatabaseError;

    return OnlineError::Okay;
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::isCorrectPassword(const QString& pw) const
  {
    if (!(hasSecretInDatabase())) return OnlineError::KeystoreEmpty;

    // initialize a PasswordProtectedSecret from the
    // encrypted data and use the built-in password checker
    const string& encData = cfgTab->operator [](CFG_KEY_KEYSTORE);
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
    const string& encData = cfgTab->operator [](CFG_KEY_KEYSTORE);
    SecretBox box(encData, true);
    bool isOkay = box.setPassword(pw.toUtf8().constData());
    if (!isOkay) return OnlineError::WrongPassword;

    // the box is unlocked. Now copy the secret signing key
    // over to our local key
    string secKeyAsString = box.getSecretAsString();
    if (secKeyAsString.empty()) return OnlineError::DatabaseError;
    secKey.fillFromString(secKeyAsString);
    secKeyUnlocked = true;

    // extract the public key from the secret key
    isOkay = cryptoLib->genPublicSignKeyFromSecretKey(secKey, pubKey);
    return isOkay ? OnlineError::Okay : OnlineError::DatabaseError;
  }

  //----------------------------------------------------------------------------

  int OnlineMngr::ping()
  {
    QString url = apiBaseUrl + "/ping";

    HttpClient cli;
    HttpResponse re = cli.blockingRequest(url, "", defaultTimeout_ms);

    return re.roundTripTime_ms;
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
    isOkay = box.setSecret(localSecKey);
    if (!isOkay) return false;

    // store the encrypted key box in our database
    string cipher = box.asString(true);
    if (cipher.empty()) return false;
    int err;
    cfgTab->set(CFG_KEY_KEYSTORE, cipher, &err);
    if (err != SQLITE_DONE) return false;
    const string& readBack = cfgTab->operator [](CFG_KEY_KEYSTORE);
    if (readBack != cipher) return false;

    // overwrite the currently used keys
    //
    // do NOT use move() here because the local keys sit on stack
    secKey.fillFromString(localSecKey.copyToString());
    pubKey.fillFromString(localPubKey.copyToString());
    secKeyUnlocked = true;

    return true;
  }

}

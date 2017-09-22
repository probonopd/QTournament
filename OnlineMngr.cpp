#include <Sloppy/json/json.h>
#include <Sloppy/Crypto/Crypto.h>
#include <Sloppy/Crypto/Sodium.h>
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
    srvPubKey.fillFromString(Sloppy::Crypto::fromBase64(ServerPubKey_B64));
  }

  //----------------------------------------------------------------------------

  OnlineError OnlineMngr::execSignedServerRequest(const QString& subUrl, const QByteArray& postData, QByteArray& responseOut)
  {
    // we need access to the secret key for signing the request
    if (!secKeyUnlocked)
    {
      return OnlineError::KeystoreLocked;
    }

    // every request will be preceeded by a 10-character
    // random nonce to avoid replay attacks
    string nonce = Sloppy::Crypto::getRandomAlphanumString(NonceLength);
    string body = nonce + string{postData.constData()};

    // create a detached signature of the body
    string sig = cryptoLib->crypto_sign_detached(body, secKey);
    string sigB64 = Sloppy::Crypto::toBase64(sig);

    // put the signature in an extra header
    QMap<QString, QString> hdr;
    hdr["X-Signature"] = QString::fromUtf8(sigB64.c_str());

    // send the request
    HttpClient cli;
    QString url = apiBaseUrl + subUrl;
    HttpResponse re = cli.blockingRequest(url, hdr, body, defaultTimeout_ms);

    // did we get a response?
    if (re.respCode < 0) return OnlineError::Timeout;
    if (re.respCode != 200) return OnlineError::BadRequest;

    // yes, check it's signature
    sigB64 = string{re.getHeader("X-Signature").toUtf8().constData()};
    if (sigB64.empty()) return OnlineError::InvalidServerSignature;
    sig = Sloppy::Crypto::fromBase64(sigB64);
    bool isOkay = cryptoLib->crypto_sign_verify_detached(re.data.constData(), sig, srvPubKey);
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
    Json::Value dic;
    dic["name"] = ord.tnmtName.toUtf8().constData();
    dic["club"] = ord.club.toUtf8().constData();
    dic["owner"] = ord.personName.toUtf8().constData();
    dic["email"] = ord.email.toUtf8().constData();
    dic["first"] = ord.firstDay.year() * 10000 + ord.firstDay.month() * 100 + ord.firstDay.day();
    dic["last"] = ord.lastDay.year() * 10000 + ord.lastDay.month() * 100 + ord.lastDay.day();
    dic["pubkey"] = Sloppy::Crypto::toBase64(pubKey.copyToString());

    // do the actual server request
    QByteArray response;
    OnlineError err = execSignedServerRequest("/registration", QByteArray(dic.toStyledString().c_str()), response);
    if (err != OnlineError::Okay) return err;

    // construct the reply
    errCodeOut.clear();

    errCodeOut = QString::fromUtf8(response.constData());
    return (errCodeOut == "OK") ? OnlineError::Okay : OnlineError::TransportOkay_AppError;
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

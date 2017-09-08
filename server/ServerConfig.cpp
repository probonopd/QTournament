
#include <boost/date_time/local_time/local_time.hpp>

#include <Sloppy/ConfigFileParser/ConfigFileParser.h>
#include <Sloppy/TemplateProcessor/TemplateSys.h>
#include <Sloppy/json/json.h>

#include <SqliteOverlay/SqliteDatabase.h>

#include "ServerConfig.h"

namespace TournamentServer
{

  ServerConfig::ServerConfig(const string& iniFilePath)
    :bindAddr{"0.0.0.0"},
      bindPort{80},
      apiBaseUrl{"/api/tournament"}
  {
    // try to open / read the server's config file
    iniFile = Sloppy::ConfigFileParser::Parser::readFromFile(iniFilePath);
    if (iniFile == nullptr)
    {
      throw invalid_argument("Could not open or read server ini file!");
    }

    // check for mandatory keys
    Sloppy::StringList keys{"Database", "TemplateDir", "TranslationStrings", "TimezoneSpec", "Timezone",
                           "CommentBodyCssClass", "SMTPServer", "SMTPUser", "SMTPPassword", "MailSender",
                           "MailRecipient", "MailUseTLS", "MailCAs", "MailTrustedServers",
                           "FQDN", "UseHTTPS"};
    for (const string& key : keys)
    {
      bool isOkay;
      string val = iniFile->getValue(key, &isOkay);

      if (!isOkay)
      {
        string msg = "Missing key '%1' in server ini file!";
        Sloppy::strArg(msg, key);
        throw std::runtime_error(msg);
      }

      if (val.empty())
      {
        string msg = "Key '%1' in server ini file has no value!";
        Sloppy::strArg(msg, key);
        throw std::runtime_error(msg);
      }
    }

    // open the database
    dbPath = iniFile->getValue("Database");
    /*db = SqliteOverlay::SqliteDatabase::get<CommentDB>(dbPath, false);
    if (db == nullptr)
    {
      string msg = "Could not open database at " + dbPath;
      throw std::runtime_error(msg);
    }*/

    // read optional keys
    bool isOkay;
    string val = iniFile->getValue("BindAddress", &isOkay);
    if (isOkay && !(val.empty())) bindAddr = val;

    val = iniFile->getValue("ApiBaseUrl", &isOkay);
    if (isOkay && !(val.empty())) apiBaseUrl = val;

    // avoid things like "http://servername//status" if we're
    // not using any API prefix path
    if (apiBaseUrl == "/") apiBaseUrl.clear();

    int newPort = iniFile->getValueAsInt("BindPort", -1, &isOkay);
    if (isOkay && (newPort > 0)) bindPort = newPort;

    // parse the templates and translations
    initTemplates();

    // prepare a mailer instance
    /*if (!(initMailer()))
    {
      throw std::runtime_error("Could not initialize SMTP mailer!");
    }*/

    // prepare the timezone file
    boost::local_time::tz_database tzdb;
    tzdb.load_from_file(iniFile->getValue("TimezoneSpec"));
    tzPtr = tzdb.time_zone_from_region(iniFile->getValue("Timezone"));
    if (tzPtr == nullptr)
    {
      string msg = "Could not initialize local time for %1 from %2";
      cerr << msg << endl;
      Sloppy::strArg(msg, iniFile->getValue("Timezone"));
      Sloppy::strArg(msg, iniFile->getValue("TimezoneSpec"));

      throw std::runtime_error(msg);
    }
  }

  //----------------------------------------------------------------------------

  string ServerConfig::getBaseUrlIncludingHost() const
  {
    bool tls = iniFile->getValueAsBool("UseHTTPS");
    string url = tls ? "https://" : "http://";
    url += iniFile->getValue("FQDN");
    url += ":" + to_string(getBindPort());
    url += getApiBaseUrl();

    return url;
  }

  //----------------------------------------------------------------------------

  void ServerConfig::initTemplates()
  {
    // read all templates from the template dir
    ts = make_unique<Sloppy::TemplateSystem::TemplateStore>(iniFile->getValue("TemplateDir"), Sloppy::StringList{});
    if (ts == nullptr)
    {
      string msg = "Error reading the template directory in " + iniFile->getValue("TemplateDir");
      cerr << msg << endl;
      throw std::runtime_error(msg);
    }

    // check for all required templates
    Sloppy::StringList tNames{"comment_section.html", "comment_snippet.html", "comments_disabled.html",
                              "admin.html", "adminPageFuncs.js", "admin_tabComments.html", "admin_tabPages.html",
                              "admin_tabCounters.html"};
    for (const string& tn : tNames)
    {
      try
      {
        string s = ts->get(tn, Json::Value{});
      }
      catch (...)
      {
        string msg = "Error parsing template " + tn + ": file not existing or bad content!";
        cerr << msg << endl;
        throw;
      }
    }

    // load the translation strings
    bool isOkay = ts->setStringlist(iniFile->getValue("TranslationStrings"));
    if (!isOkay)
    {
      string msg = "Error loading translation strings from " + iniFile->getValue("TranslationStrings");
      cerr << msg << endl;
      throw std::runtime_error(msg);
    }
  }

  //----------------------------------------------------------------------------

  /*bool ServerConfig::initMailer()
  {
    string smtpUrl = "smtp://" + iniFile->getValue("SMTPServer");
    bool useTls = iniFile->getValueAsBool("MailUseTLS");

    auto tmpMailer = make_unique<SMTP_Mailer>(smtpUrl, useTls,
                                      iniFile->getValue("SMTPUser"),
                                      iniFile->getValue("SMTPPassword"));

    if (tmpMailer == nullptr) return false;

    if (useTls)
    {
      Sloppy::StringList ca;
      Sloppy::stringSplitter(ca, iniFile->getValue("MailCAs"), ";", true);

      Sloppy::StringList trusted;
      Sloppy::stringSplitter(trusted, iniFile->getValue("MailTrustedServers"), ";", true);

      bool isOk = tmpMailer->prepCerts(ca, trusted);
      if (!isOk) return false;
    }

    mailer = move(tmpMailer);
    return true;
  }*/

}

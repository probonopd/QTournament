#ifndef BLOG_COMMENT_SYS__SERVER_CONFIG_H
#define BLOG_COMMENT_SYS__SERVER_CONFIG_H

#include <memory>

#include <boost/date_time/local_time/local_time.hpp>

#include <Sloppy/TemplateProcessor/TemplateSys.h>

//#include "SMTP_Mailer.h"

using namespace std;

// forward
namespace Sloppy
{
  namespace ConfigFileParser {
    class Parser;
  }

  namespace TemplateSystem {
    class TemplateStore;
  }
}


namespace TournamentServer
{
  // forward
  //class CommentDB;
  //class SMTP_Mailer;

  class ServerConfig
  {
  public:
    ServerConfig(const string& iniFilePath);

    string getBindAddr() const { return bindAddr; }
    int getBindPort() const { return bindPort; }
    string getApiBaseUrl() const { return apiBaseUrl; }
    string getDatabasePath() const { return dbPath; }
    //CommentDB* getDb() const { return db.get(); }
    Sloppy::TemplateSystem::TemplateStore* getTemplateStore() const { return ts.get(); }
    const boost::local_time::time_zone_ptr& getTimezonePtrRef() const { return tzPtr; }

    string getCommentBodyCssClass() const { return iniFile->getValue("CommentBodyCssClass"); }

    //SMTP_Mailer* getMailer() const { return mailer.get(); }
    string getMailSender() const { return iniFile->getValue("MailSender"); }
    string getMailRecipient() const { return iniFile->getValue("MailRecipient"); }

    string getBaseUrlIncludingHost() const;

  protected:
    void initTemplates();
    //bool initMailer();

  private:
    string bindAddr;
    int bindPort;
    string apiBaseUrl;
    string dbPath;
    unique_ptr<Sloppy::ConfigFileParser::Parser> iniFile;
    //unique_ptr<CommentDB> db;
    unique_ptr<Sloppy::TemplateSystem::TemplateStore> ts;
    boost::local_time::time_zone_ptr tzPtr;
    //unique_ptr<SMTP_Mailer> mailer;
  };
}

#endif

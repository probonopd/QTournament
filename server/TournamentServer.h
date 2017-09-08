#ifndef BLOG_COMMENT_SYS__SERVER_H
#define BLOG_COMMENT_SYS__SERVER_H

#include <MongooseWrapper/MongooseWrapper.h>
#include <MongooseWrapper/HttpProtocol.h>
#include <MongooseWrapper/HttpServer.h>

// forward
namespace Sloppy
{
  namespace TemplateSystem
  {
    class TemplateStore;
  }
}

namespace TournamentServer
{
  // forward
  class CommentDB;
  class ServerConfig;
  class Comment;

  class Server : public Mongoose::StatelessHttpServer
  {
  public:
    Server(ServerConfig* _cfg);

  protected:
    Mongoose::HttpResponseBuilder statusHandler(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder getCommentHtmlHandler(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder submitCommentHandler(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder adminPageHandler(http_message* msg, const string& remainder, const string& remoteIP);

    Mongoose::HttpResponseBuilder getLast20JSON(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder updateCommentState(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder addPage(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder updatePageState(http_message* msg, const string& remainder, const string& remoteIP);

    Mongoose::HttpResponseBuilder getAllPagesJSON(http_message* msg, const string& remainder, const string& remoteIP);

    Mongoose::HttpResponseBuilder getServerStats(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder getViewCounts(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder getTotalCounts(http_message* msg, const string& remainder, const string& remoteIP);

    Mongoose::HttpResponseBuilder sendTestEmail(http_message* msg, const string& remainder, const string& remoteIP);
    Mongoose::HttpResponseBuilder execTokenAction(http_message* msg, const string& remainder, const string& remoteIP);


    Mongoose::HttpResponseBuilder handleCORS(const Mongoose::HttpRequestParser& req);

    string renderCommentSection(const string& articleName);
    string renderSingleComment(const Comment& c, int idx);

    void sendMailUponCommentSubmission(const Comment& c) const;

  private:
    ServerConfig* cfg;
    CommentDB* db;
    Sloppy::TemplateSystem::TemplateStore* ts;
  };


}
#endif

#include <Sloppy/TemplateProcessor/TemplateSys.h>
#include <Sloppy/json/json.h>

#include "TournamentServer.h"
#include "ServerConfig.h"

namespace TournamentServer
{
  Server::Server(ServerConfig* _cfg)
    :Mongoose::StatelessHttpServer{}, cfg{_cfg}
  {
    // make sure we have a valid handle
    if (cfg == nullptr)
    {
      throw invalid_argument{"CommentServer: got nullptr for database!"};
    }
    //db = cfg->getDb();
    ts = cfg->getTemplateStore();

    string baseUrl = cfg->getApiBaseUrl();
    registerHandler(baseUrl + "/ping", bind(&Server::statusHandler, this, placeholders::_1, placeholders::_2, placeholders::_3));
  }

  //----------------------------------------------------------------------------

  Mongoose::HttpResponseBuilder Server::statusHandler(http_message* msg, const string& remainder, const string& remoteIP)
  {
    // we don't expect any additional URL parts
    if (!(remainder.empty()))
    {
      return Mongoose::HttpResponseBuilder{400, "Bad Request", true};
    }

    Mongoose::HttpResponseBuilder resp{200, "OK", true};
    resp.setBody("pong");

    cout << "Received ping request!" << endl;

    return resp;
  }

}

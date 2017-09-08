#include <iostream>

#include <Sloppy/libSloppy.h>
#include <Sloppy/ConfigFileParser/ConfigFileParser.h>

#include "TournamentServer.h"
#include "ServerConfig.h"
//#include "SMTP_Mailer.h"

using namespace std;

int main(int argc, char** argv)
{
  // we need one argument: the path of the server ini file
  if (argc != 2)
  {
    cerr << "Need exactly one argument: the path to the ini file!" << endl;
    return 1;
  }
  string iniPath = argv[1];

  // try to parse the config file
  unique_ptr<TournamentServer::ServerConfig> cfg;
  try
  {
    cfg = make_unique<TournamentServer::ServerConfig>(iniPath);
  }
  catch (...)
  {
    cerr << "Error reading the ini file!" << endl;
    return 1;
  }

  // create a server
  Mongoose::Manager* mgr = Mongoose::Manager::getInstance();
  TournamentServer::Server srv{cfg.get()};
  string bind = "%1:%2";
  Sloppy::strArg(bind, cfg->getBindAddr());
  Sloppy::strArg(bind, cfg->getBindPort());
  auto srvConn = mgr->bindStateless(bind, &srv);
  if (srvConn == nullptr)
  {
    cerr << "Could not start server on " << bind << endl;
    return 1;
  }

  cout << "Server config" << endl;
  cout << "  DB = " << cfg->getDatabasePath() << endl;
  cout << "  Server address = " << cfg->getBindAddr() << endl;
  cout << "  Server port = " << cfg->getBindPort() << endl;
  cout << "  API Base URL = " << cfg->getApiBaseUrl() << endl;

  cout << endl << endl << "Serving..." << endl;

  // run the event loop
  mgr->runBlocking(100);

  // we'll never reach this point
  return 0;
}

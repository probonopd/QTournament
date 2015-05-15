#include "SignalRelay.h"

SignalRelay* SignalRelay::instance = nullptr;

SignalRelay::SignalRelay(QObject *parent) : QObject(parent),
  matchTabView_Sender(nullptr)
{

}

//----------------------------------------------------------------------------

void SignalRelay::checkConnection()
{
  if (matchTabView_Sender != nullptr)
  {
    while(!(receiverQueue.isEmpty()))
    {
      ResultSheets* r = receiverQueue.dequeue();
      connect(matchTabView_Sender, SIGNAL(matchSelectionChanged(int)), r, SLOT(onMatchSelectionChanged(int)));
    }
  }
}

//----------------------------------------------------------------------------

void SignalRelay::registerSender(MatchTableView* s)
{
  // we have only one source
  if ((s != nullptr) && (matchTabView_Sender == nullptr))
  {
    matchTabView_Sender = s;    
    checkConnection();
  }
}

//----------------------------------------------------------------------------

void SignalRelay::registerReceiver(ResultSheets* r)
{
  if (r != nullptr)
  {
    receiverQueue.append(r);
    checkConnection();
  }
}

//----------------------------------------------------------------------------

void SignalRelay::cleanUp()
{
  if (instance != nullptr)
  {
    delete instance;
  }
}

//----------------------------------------------------------------------------

SignalRelay* SignalRelay::getInstance()
{
  if (instance == nullptr)
  {
    instance = new SignalRelay();
  }

  return instance;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


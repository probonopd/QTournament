/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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


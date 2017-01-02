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

#ifndef SIGNALRELAY_H
#define SIGNALRELAY_H

#include <QObject>
#include <QQueue>

#include "ui/MatchTableView.h"
#include "reports/ResultSheets.h"

class SignalRelay : public QObject
{
  Q_OBJECT
public:
  static SignalRelay* getInstance();
  void registerSender(MatchTableView* s);
  void registerReceiver(ResultSheets* r);
  static void cleanUp();

private:
  explicit SignalRelay(QObject *parent = 0);
  static SignalRelay* instance;
  MatchTableView* matchTabView_Sender;
  QQueue<ResultSheets*> receiverQueue;
  void checkConnection();

signals:

public slots:
};

#endif // SIGNALRELAY_H

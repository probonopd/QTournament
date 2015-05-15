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

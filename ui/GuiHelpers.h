#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QObject>
#include <QString>

#include "Match.h"
#include "Court.h"

class GuiHelpers : public QObject
{
  Q_OBJECT

public:
  GuiHelpers();
  ~GuiHelpers();
  static QString groupNumToString(int grpNum);
  static QString prepCall(const QTournament::Match& ma, const QTournament::Court& co);
};

#endif // GUIHELPERS_H

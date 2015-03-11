#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QPainter>
#include <QFont>
#include <QColor>

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

  static void drawFormattedText(QPainter* painter, QRect r, const QString& s, int alignmentFlags=Qt::AlignVCenter|Qt::AlignLeft,
                                bool isBold=false, bool isItalics=false, QFont fnt=QFont(), QColor fntColor = QColor(0,0,0),
                                double fntSizeFac = 1.0);
  static void drawFormattedText(QPainter* painter, QRect r, const QString& s, int alignmentFlags=Qt::AlignVCenter|Qt::AlignLeft,
                                bool isBold=false, bool isItalics=false, double fntSizeFac = 1.0);
};

#endif // GUIHELPERS_H

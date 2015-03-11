#include "GuiHelpers.h"

#include "TournamentDataDefs.h"

GuiHelpers::GuiHelpers()
{

}

GuiHelpers::~GuiHelpers()
{

}

QString GuiHelpers::groupNumToString(int grpNum)
{
  if (grpNum > 0) return QString::number(grpNum);

  switch (grpNum)
  {
  case GROUP_NUM__ITERATION:
    return "--";
  case GROUP_NUM__L16:
    return tr("L16");
  case GROUP_NUM__QUARTERFINAL:
    return tr("QF");
  case GROUP_NUM__SEMIFINAL:
    return tr("SF");
  case GROUP_NUM__FINAL:
    return tr("FI");
  }

  return "??";

}

/**
 * Generates a text proposal that the user should announce when calling a match
 *
 * This can't be integrated in the Match class, because Match is not derived
 * from QObject, so we don't have tr() available there
 *
 * @param ma the match to call
 * @param co the court the match shall be played on
 *
 * @return a string with the announcement
 */
QString GuiHelpers::prepCall(const QTournament::Match &ma, const QTournament::Court &co)
{
  QString call = tr("Please announce:\n\n\n");

  call += tr("Next match,\n\n");
  call += tr("match number ") + QString::number(ma.getMatchNumber()) + " on court number " + QString::number(co.getNumber());
  call += "\n\n";
  call += ma.getCategory().getName() + ",\n\n";
  call += ma.getPlayerPair1().getCallName(tr("and")) + "\n\n";
  call += tr("        versus\n\n");
  call += ma.getPlayerPair2().getCallName(tr("and")) + ",\n\n";
  call += tr("match number ") + QString::number(ma.getMatchNumber()) + " on court number " + QString::number(co.getNumber());
  call += "\n\n\n";
  call += tr("Call executed?");

  return call;
}


void GuiHelpers::drawFormattedText(QPainter *painter, QRect r, const QString &s, int alignmentFlags, bool isBold, bool isItalics, QFont fnt, QColor fntColor, double fntSizeFac)
{
  fnt.setItalic(isItalics);
  fnt.setBold(isBold);
  fnt.setPointSizeF(fnt.pointSizeF() * fntSizeFac);

  painter->save();
  painter->setPen(QPen(fntColor));
  painter->setFont(fnt);
  painter->drawText(r, alignmentFlags, s);
  painter->restore();
}

void GuiHelpers::drawFormattedText(QPainter *painter, QRect r, const QString &s, int alignmentFlags, bool isBold, bool isItalics, double fntSizeFac)
{
  drawFormattedText(painter, r, s, alignmentFlags, isBold, isItalics, QFont(), QColor(0,0,0), fntSizeFac);
}

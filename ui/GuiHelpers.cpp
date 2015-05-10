#include "GuiHelpers.h"

#include <QMessageBox>

#include "TournamentDataDefs.h"
#include "MatchMngr.h"
#include "Tournament.h"

GuiHelpers::GuiHelpers()
{

}

GuiHelpers::~GuiHelpers()
{

}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

QString GuiHelpers::groupNumToLongString(int grpNum)
{
  if (grpNum > 0) return (tr("Group ") + QString::number(grpNum));

  switch (grpNum)
  {
  case GROUP_NUM__ITERATION:
    return "--";
  case GROUP_NUM__L16:
    return tr("Round of Last 16");
  case GROUP_NUM__QUARTERFINAL:
    return tr("Quarter Finals");
  case GROUP_NUM__SEMIFINAL:
    return tr("Semi Finals");
  case GROUP_NUM__FINAL:
    return tr("Finals");
  }

  return "??";

}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

void GuiHelpers::drawFormattedText(QPainter *painter, QRect r, const QString &s, int alignmentFlags, bool isBold, bool isItalics, double fntSizeFac)
{
  drawFormattedText(painter, r, s, alignmentFlags, isBold, isItalics, QFont(), QColor(0,0,0), fntSizeFac);
}

//----------------------------------------------------------------------------

void GuiHelpers::execWalkover(const QTournament::Match& ma, int playerNum)
{
  if ((playerNum != 1) && (playerNum != 2)) return; // shouldn't happen
  if (!(ma.isWalkoverPossible())) return;

  // get a user confirmation
  QString msg = tr("This will be a walkover for\n\n\t");
  if (playerNum == 1)
  {
    msg += ma.getPlayerPair1().getDisplayName();
  } else {
    msg += ma.getPlayerPair2().getDisplayName();
  }
  msg += "\n\n";
  msg += tr("All games will be 21:0.") + "\n\n";
  msg += tr("WARNING: this step is irrevocable!") + "\n\n";
  msg += tr("Proceed?");
  int result = QMessageBox::question(0, tr("Confirm walkover"), msg);
  if (result != QMessageBox::Yes)
  {
    return;
  }
  QTournament::MatchMngr* mm = QTournament::Tournament::getMatchMngr();
  assert(mm != nullptr);
  mm->walkover(ma, playerNum);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


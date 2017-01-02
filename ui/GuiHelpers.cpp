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

#include "GuiHelpers.h"

#include <QMessageBox>
#include <QFontMetricsF>

#include "TournamentDataDefs.h"
#include "MatchMngr.h"

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
QString GuiHelpers::prepCall(const QTournament::Match &ma, const QTournament::Court &co, int nCall)
{
  QString call = tr("Please announce:\n\n\n");

  if (nCall == 0)
  {
    call += tr("Next match,\n\n");
  } else {
    call += QString::number(nCall + 1) + ". ";
    call += tr("call for ");
  }

  call += tr("match number ") + QString::number(ma.getMatchNumber()) + tr(" on court number ") + QString::number(co.getNumber());
  call += "\n\n";
  call += ma.getCategory().getName() + ",\n\n";
  call += ma.getPlayerPair1().getCallName(tr("and")) + "\n\n";
  call += tr("        versus\n\n");
  call += ma.getPlayerPair2().getCallName(tr("and")) + ",\n\n";
  call += tr("match number ") + QString::number(ma.getMatchNumber()) + tr(" on court number ") + QString::number(co.getNumber());
  call += ".";

  // add the umpire's name, if necessary
  QTournament::REFEREE_MODE refMode = ma.get_EFFECTIVE_RefereeMode();
  if ((refMode != QTournament::REFEREE_MODE::NONE) && ((refMode != QTournament::REFEREE_MODE::HANDWRITTEN)))
  {
    QTournament::upPlayer referee = ma.getAssignedReferee();
    if (referee != nullptr)
    {
      call += "\n\n\n";
      call += tr("Umpire is ");
      call += referee->getDisplayName_FirstNameFirst();
      call += ".";
    }
  }

  // add additional calls, if applicable
  if (nCall > 0)
  {
    call += "\n\n";
    call += tr("THIS IS THE ");
    call += QString::number(nCall + 1) + ". ";
    call += tr("CALL!");
  }
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

void GuiHelpers::drawFormattedText(QPainter* painter, int x0, int yBaseline, const QString& s, bool isBold, bool isItalics, QFont fnt, QColor fntColor, double fntSizeFac)
{
  fnt.setItalic(isItalics);
  fnt.setBold(isBold);
  fnt.setPointSizeF(fnt.pointSizeF() * fntSizeFac);

  painter->save();
  painter->setPen(QPen(fntColor));
  painter->setFont(fnt);
  painter->drawText(x0, yBaseline, s);
  painter->restore();
}

//----------------------------------------------------------------------------

void GuiHelpers::drawFormattedText(QPainter *painter, QRect r, const QString &s, int alignmentFlags, bool isBold, bool isItalics, double fntSizeFac)
{
  drawFormattedText(painter, r, s, alignmentFlags, isBold, isItalics, QFont(), QColor(0,0,0), fntSizeFac);
}

//----------------------------------------------------------------------------

QSizeF GuiHelpers::getFormattedTextSize(QPainter* painter, const QString& s, bool isBold, bool isItalics, QFont fnt, double fntSizeFac)
{
  fnt.setItalic(isItalics);
  fnt.setBold(isBold);
  fnt.setPointSizeF(fnt.pointSizeF() * fntSizeFac);

  QFontMetricsF fm{fnt};
  return fm.boundingRect(s).size();
}

//----------------------------------------------------------------------------

QSizeF GuiHelpers::drawTwoLinePlayerPairNames(QPainter* painter, int topLeftX, int topLeftY, const QTournament::Match& ma,
                                              const QString& localWinnerName, const QString localLoserName,
                                              double percLineSpace, bool isBold, bool isItalics, QFont fnt, QColor fntColor, double fntSizeFac)
{
  //
  // step 1: get the text items to be drawn. The overall layout is as follows
  //
  //               row1Left : row1Right
  //               row2Left   row2Right
  //
  // where the left text block is right aligned and the right text block is left aligned
  //
  QString row1Left;
  QString row2Left;
  QString row1Right;
  QString row2Right;
  bool isDoubles;
  ma.getDisplayNameTextItems(localWinnerName, localLoserName, row1Left, row2Left, row1Right, row2Right, isDoubles);

  //
  // now do the actual drawing
  //
  // initial note: we don't call drawFormattedText() here to avoid double
  // initialization of font objects, font metrics etc.
  //

  // prepare the font
  fnt.setItalic(isItalics);
  fnt.setBold(isBold);
  fnt.setPointSizeF(fnt.pointSizeF() * fntSizeFac);

  // prepare font metrics
  QFontMetricsF fm{fnt};

  // prepare the paint device
  painter->save();
  painter->setPen(QPen(fntColor));
  painter->setFont(fnt);

  // prepare a flag that indicates that we have
  // at least one block with two lines
  bool hasTwoLines = false;

  // convert topLeftY to the baseline position
  double yBaseline1 = topLeftY + fm.ascent();
  double yBaseline2 = yBaseline1 + fm.height() * (1 + percLineSpace);

  //
  // draw the left block right aligned
  //
  // if this is not a doubles pair, our job is easy
  double maxX = -1;
  if (row2Left.isEmpty())
  {
    painter->drawText(topLeftX, yBaseline1, row1Left);
    maxX = topLeftX + fm.width(row1Left);
  } else {
    // okay, two players in this pair.
    // determine the top right corner of the text
    double width1 = fm.width(row1Left);
    double width2 = fm.width(row2Left);
    double maxWidth = (width1 > width2) ? width1 : width2;
    maxX = topLeftX + maxWidth;

    // determine the top left corner of the first and second line
    double topLeftX1 = maxX - width1;
    double topLeftX2 = maxX - width2;

    // actually draw the text
    painter->drawText(topLeftX1, yBaseline1, row1Left);
    painter->drawText(topLeftX2, yBaseline2, row2Left);

    // set the two-lines-flag
    hasTwoLines = true;
  }

  // draw the colon in row 1
  QString colon = " : ";
  painter->drawText(maxX, yBaseline1, colon);
  maxX += fm.width(colon);

  //
  // draw the right block left aligned
  //
  // this is easy, because left-alignment is the default
  painter->drawText(maxX, yBaseline1, row1Right);
  double width1 = fm.width(row1Right);

  double width2 = -1;
  if (!(row2Right.isEmpty()))
  {
    painter->drawText(maxX, yBaseline2, row2Right);
    width2 = fm.width(row1Right);

    // set the two-lines-flag
    hasTwoLines = true;
  }
  maxX += (width1 > width2) ? width1 : width2;

  // we're done with painting
  painter->restore();

  //
  // determine the overall extensions of the text block
  //
  double h = hasTwoLines ? fm.height() * (2 + percLineSpace) : fm.height();
  double w = maxX - topLeftX;
  return QSizeF(w, h);
}

//----------------------------------------------------------------------------

void GuiHelpers::drawTwoLinePlayerPairNames_Centered(QPainter* painter, const QRectF rect, const QTournament::Match& ma,
                                                     const QString& localWinnerName, const QString localLoserName,
                                                     double percLineSpace, bool isBold, bool isItalics, QFont fnt, QColor fntColor, double fntSizeFac)
{
  //
  // step 1: get the text items to be drawn. The overall layout is as follows
  //
  //               row1Left : row1Right
  //               row2Left   row2Right
  //
  // where the left text block is right aligned and the right text block is left aligned
  // and everything is horizontally and vertically centered in rect
  //
  QString row1Left;
  QString row2Left;
  QString row1Right;
  QString row2Right;
  bool isDoubles;
  ma.getDisplayNameTextItems(localWinnerName, localLoserName, row1Left, row2Left, row1Right, row2Right, isDoubles);

  //
  // now do the actual drawing
  //
  // initial note: we don't call drawFormattedText() here to avoid double
  // initialization of font objects, font metrics etc.
  //

  // prepare the font
  fnt.setItalic(isItalics);
  fnt.setBold(isBold);
  fnt.setPointSizeF(fnt.pointSizeF() * fntSizeFac);

  // prepare font metrics
  QFontMetricsF fm{fnt};

  // prepare the paint device
  painter->save();
  painter->setPen(QPen(fntColor));
  painter->setFont(fnt);

  // prepare a flag that indicates that we have
  // at least one block with two lines
  bool hasTwoLines = !(row2Left.isEmpty() && row2Right.isEmpty());

  // determine the overall text width
  QString colon = " : ";
  double r1LeftWidth = fm.width(row1Left);
  double r2LeftWidth = fm.width(row2Left);
  double r1RightWidth = fm.width(row1Right);
  double r2RightWidth = fm.width(row2Right);
  double maxLeftWidth = max(r1LeftWidth, r2LeftWidth);
  double maxRightWidth = max(r1RightWidth, r2RightWidth);
  double totalWidth = maxLeftWidth + fm.width(colon) + maxRightWidth;

  // calculate the side margin
  double horMargin = (rect.width() - totalWidth) / 2.0;
  if (horMargin < 0) horMargin = 0;

  // calculate the top/bottom margin
  double textHeight = hasTwoLines ? fm.height() * (2 + percLineSpace) : fm.height();
  double vertMargin = (rect.height() - textHeight) / 2.0;
  if (vertMargin < 0) vertMargin = 0;

  // calculate the top left x positions of all four items
  double r1LeftX0 = rect.x() + horMargin + maxLeftWidth - r1LeftWidth;
  double r2LeftX0 = rect.x() + horMargin + maxLeftWidth - r2LeftWidth;
  double rightX0 = rect.x() + rect.width() - horMargin - maxRightWidth;  // is identical for both rows ==> left-alignment!

  // calculate the top lext x position of the colon string
  double colonLeftX0 = rect.x() + horMargin + maxLeftWidth;

  // calculate the baseline positions
  double yBaseline1 = rect.y() + vertMargin + fm.ascent();
  double yBaseline2 = yBaseline1 + (1 + percLineSpace) * fm.height();

  //
  // let the drawing begin
  //
  // draw row 1
  painter->drawText(r1LeftX0, yBaseline1, row1Left);
  painter->drawText(rightX0, yBaseline1, row1Right);

  // draw row 2, if any
  if (!(row2Left.isEmpty()))
  {
    painter->drawText(r2LeftX0, yBaseline2, row2Left);
  }
  if (!(row2Right.isEmpty()))
  {
    painter->drawText(rightX0, yBaseline2, row2Right);
  }

  // draw the colon
  painter->drawText(colonLeftX0, yBaseline1, colon);

  // done
  painter->restore();
}

//----------------------------------------------------------------------------

void GuiHelpers::execWalkover(QWidget* parent, const QTournament::Match& ma, int playerNum)
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
  int result = QMessageBox::question(parent, tr("Confirm walkover"), msg);
  if (result != QMessageBox::Yes)
  {
    return;
  }
  QTournament::MatchMngr mm{ma.getDatabaseHandle()};
  mm.walkover(ma, playerNum);
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


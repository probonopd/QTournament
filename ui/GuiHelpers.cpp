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

#include <QObject>
#include <QMessageBox>
#include <QFontMetricsF>
#include <QLocale>
#include <QFile>

#include "TournamentDataDefs.h"
#include "MatchMngr.h"
#include "TournamentDataDefs.h"
#include "Match.h"
#include "Court.h"
#include "PlayerPair.h"
#include "Player.h"
#include "PlayerProfile.h"

using namespace QTournament;

QString GuiHelpers::groupNumToString(int grpNum)
{
  if (grpNum > 0) return QString::number(grpNum);

  switch (grpNum)
  {
  case GroupNum_Iteration:
    return "--";
  case GroupNum_L16:
    return QObject::tr("L16");
  case GroupNum_Quarter:
    return QObject::tr("QF");
  case GroupNum_Semi:
    return QObject::tr("SF");
  case GroupNum_Final:
    return QObject::tr("FI");
  }

  return "??";

}

//----------------------------------------------------------------------------

QString GuiHelpers::groupNumToLongString(int grpNum)
{
  if (grpNum > 0) return (QObject::tr("Group ") + QString::number(grpNum));

  switch (grpNum)
  {
  case GroupNum_Iteration:
    return "--";
  case GroupNum_L16:
    return QObject::tr("Round of Last 16");
  case GroupNum_Quarter:
    return QObject::tr("Quarter Finals");
  case GroupNum_Semi:
    return QObject::tr("Semi Finals");
  case GroupNum_Final:
    return QObject::tr("Finals");
  }

  return "??";

}

//----------------------------------------------------------------------------

/**
 * Generates a text proposal that the user should announce when calling a match
 *
 * @param ma the match to call
 * @param co the court the match shall be played on
 *
 * @return a string with the announcement
 */
QString GuiHelpers::prepCall(const QTournament::Match &ma, const QTournament::Court &co, int nCall)
{
  int maNum = ma.getMatchNumber();
  int coNum = co.getNumber();

  QString call = "<i><font color=\"blue\">" + QObject::tr("Please announce:") + "</font></i><br><br><br>";

  call += "<big>";

  if (nCall == 0)
  {
    call += QObject::tr("Next match,<br><br>");
  } else {
    call += "<b><font color=\"darkRed\">%1. ";
    call = call.arg(QString::number(nCall + 1));
    call += QObject::tr("call");
    call += "</font></b>";
    call += QObject::tr(" for ");
  }

  call += QObject::tr("match number ") + "<b><font color=\"darkRed\">%1</font></b>";
  call += QObject::tr(" on court number ") + "<b><font color=\"darkRed\">%2</font></b><br><br>";
  call = call.arg(maNum);
  call = call.arg(coNum);

  call += "<center><b>";

  call += ma.getCategory().getName() + ",<br><br>";

  int winnerRank = ma.getWinnerRank();
  if (winnerRank > 0)
  {
    call += "<font color=\"darkRed\">";
    if (winnerRank == 1)
    {
      call += QObject::tr("KO_Start::Final");
    } else {
      call += QObject::tr("MATCH FOR PLACE %1");
      call = call.arg(winnerRank);
    }
    call += "</font>";
    call +=  ",<br><br>";
  }

  call += ma.getPlayerPair1().getCallName(QObject::tr("and")) + "<br><br>";
  call += QObject::tr("versus<br><br>");
  call += ma.getPlayerPair2().getCallName(QObject::tr("and")) + ",<br><br></b></center>";
  call += QObject::tr("match number ") + "<b><font color=\"darkRed\">%1</font></b>";
  call += QObject::tr(" on court number ") + "<b><font color=\"darkRed\">%2</font></b>";
  call = call.arg(maNum);
  call = call.arg(coNum);
  call += ".";

  // add the umpire's name, if necessary
  QTournament::RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
  if ((refMode != QTournament::RefereeMode::None) && ((refMode != QTournament::RefereeMode::HandWritten)))
  {
    auto referee = ma.getAssignedReferee();
    if (referee)
    {
      call += "<br><br><br><b>";
      call += QObject::tr("Umpire is %1.");
      call = call.arg(referee->getDisplayName_FirstNameFirst());
      call += "</b><br>";
    }
  }

  // add additional calls, if applicable
  if (nCall > 0)
  {
    call += "<br><br>";
    call += "<b><font color=\"darkRed\">";
    call += QObject::tr("THIS IS THE %1. CALL!");
    call += "</font></b>";
    call = call.arg(nCall + 1);
  }
  call += "</big><br><br><br>";
  call += "<i><font color=\"blue\">" + QObject::tr("Call executed?") + "</font></i><br><br><br>";

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
  ma.getDisplayNameTextItems(localWinnerName, localLoserName, row1Left, row2Left, row1Right, row2Right);

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
                                                     double percLineSpace, bool isBold, bool isItalics, QFont fnt, QColor fntColor, double fntSizeFac,
                                                     QColor winnerNameColor, QColor loserNameColor)
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
  ma.getDisplayNameTextItems(localWinnerName, localLoserName, row1Left, row2Left, row1Right, row2Right);

  // determine the colors for the left and the right block
  QColor leftColor{fntColor};
  QColor rightColor{fntColor};
  if (ma.isInState(ObjState::MA_Finished))
  {
    auto w = ma.getWinner();
    auto l = ma.getLoser();

    if (w && l)
    {
      auto pp1 = ma.getPlayerPair1();
      if (*w == pp1)
      {
        leftColor = winnerNameColor;
        rightColor = loserNameColor;
      } else {
        leftColor = loserNameColor;
        rightColor = winnerNameColor;
      }
    }
  }

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
  double maxLeftWidth = std::max(r1LeftWidth, r2LeftWidth);
  double maxRightWidth = std::max(r1RightWidth, r2RightWidth);
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

  // draw the left block
  painter->setPen(QPen(leftColor));
  painter->drawText(r1LeftX0, yBaseline1, row1Left);
  if (!(row2Left.isEmpty()))
  {
    painter->drawText(r2LeftX0, yBaseline2, row2Left);
  }

  // draw the right block
  painter->setPen(QPen(rightColor));
  painter->drawText(rightX0, yBaseline1, row1Right);
  if (!(row2Right.isEmpty()))
  {
    painter->drawText(rightX0, yBaseline2, row2Right);
  }

  // draw the colon
  painter->setPen(QPen(fntColor));
  painter->drawText(colonLeftX0, yBaseline1, colon);

  // done
  painter->restore();
}

//----------------------------------------------------------------------------

QString GuiHelpers::getStatusSummaryForPlayer(const QTournament::Player& p)
{
  QTournament::PlayerProfile pp{p};
  return getStatusSummaryForPlayer(p, pp);
}

//----------------------------------------------------------------------------

QString GuiHelpers::getStatusSummaryForPlayer(const QTournament::Player& p, const QTournament::PlayerProfile& pp)
{
  using namespace QTournament;

  QTournament::ObjState plStat = p.getState();

  QString txt;
  if (plStat == QTournament::ObjState::PL_Idle)
  {
    txt = QObject::tr(" is idle");

    auto ma = pp.getLastPlayedMatch();
    if (ma)
    {
      txt += QObject::tr(". The last match ended %1 ago.");
      txt = txt.arg(qdt2durationString(ma->getFinishTime()));
    } else {
      txt += QObject::tr("; no played matches yet.");
    }
  }
  std::optional<Match> ma;
  if ((plStat == QTournament::ObjState::PL_Playing) ||
      (plStat == QTournament::ObjState::PL_Referee))
  {
    if (plStat == QTournament::ObjState::PL_Playing)
    {
      txt = QObject::tr(" is playing on court %1 for %2 (match %3, %4, Round %5)");
      ma = pp.getCurrentMatch();
    }
    if (plStat == QTournament::ObjState::PL_Referee)
    {
      txt = QObject::tr(" is umpire on court %1 for %2 (match %3, %4, Round %5)");
      ma = pp.getCurrentUmpireMatch();
    }

    if (ma)
    {
      auto co = ma->getCourt(nullptr);
      txt = txt.arg(co ? QString::number(co->getNumber()) : "??");

      QDateTime sTime = ma->getStartTime();
      txt = txt.arg(sTime.isValid() ? qdt2durationString(sTime) : "??");

      txt = txt.arg(ma->getMatchNumber());
      txt = txt.arg(ma->getCategory().getName());
      txt = txt.arg(ma->getMatchGroup().getRound());
    } else {
      txt = "Waaaaah!!! Database inconsistency!!! Panic!!";
    }
  }
  if (plStat == QTournament::ObjState::PL_WaitForRegistration)
  {
    txt = QObject::tr(" has not yet shown up for registration.");
  }

  return txt;
}

//----------------------------------------------------------------------------

QString GuiHelpers::qdt2durationString(const QDateTime& qdt)
{
  const time_t now = QDateTime::currentDateTimeUtc().toTime_t();

  time_t other = qdt.toTime_t();
  int duration = abs(now - other);

  int hours = duration / 3600;
  int minutes = (duration % 3600) / 60;
  QString sDuration = "%1:%2";
  sDuration = sDuration.arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));

  return sDuration;
}

//----------------------------------------------------------------------------

QString GuiHelpers::qdt2string(const QDateTime& qdt)
{
  return qdt.toString("HH:mm");
}

//----------------------------------------------------------------------------

QString GuiHelpers::getLocaleDependedResource(const QString& resName)
{
  // determine the local as a string and check if we're
  // using a German locale
  bool isGerman = QLocale().name().startsWith("de", Qt::CaseInsensitive);

  // determine the local resource name
  QString rn{resName};
  if (isGerman)
  {
    // find the file extension (==> everything after and including the last dot)
    auto idxLastDot = resName.lastIndexOf('.');
    QString ext{};
    if (idxLastDot >= 0)
    {
      ext = resName.mid(idxLastDot);  // includes the dot
      rn.truncate(idxLastDot);
    }
    rn += "_de" + ext;

    // fall back to the default if no local version exists
    if (!QFile::exists(rn)) rn = resName;
  }

  QFile binData{rn};
  if (!binData.open(QIODevice::ReadOnly)) return QString{};

  return binData.readAll();
}

//----------------------------------------------------------------------------

bool GuiHelpers::showAndConfirmMatchResult(QWidget* parent, const Match& ma, const std::optional<MatchScore>& matchResult)
{
  auto pp1Name = ma.getPlayerPair1().getDisplayName();
  auto pp2Name = ma.getPlayerPair2().getDisplayName();
  QString sResult = matchResult->toString();
  sResult = sResult.replace(",", "  ,  ");
  QString msg = getLocaleDependedResource(":/ui/match_result_confirmation.html");
  msg = msg.arg(pp1Name);
  msg = msg.arg(pp2Name);
  msg = msg.arg(sResult);
  if (matchResult->getWinner() == 1)
  {
    msg = msg.arg(pp1Name);
    msg = msg.arg(pp2Name);
  }
  if (matchResult->getWinner() == 2)
  {
    msg = msg.arg(pp2Name);
    msg = msg.arg(pp1Name);
  }
  if (matchResult->getWinner() == 0)
  {
    msg = msg.arg(QObject::tr("--- (Draw)"));
    msg = msg.arg(QObject::tr("--- (Draw)"));
  }

  QMessageBox dlgConfirm{parent};
  dlgConfirm.setText(msg);
  dlgConfirm.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  dlgConfirm.setDefaultButton(QMessageBox::Yes);
  dlgConfirm.setWindowTitle(QObject::tr("Please confirm match result"));

  return (dlgConfirm.exec() == QMessageBox::Yes);
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




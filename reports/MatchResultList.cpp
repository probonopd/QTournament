#include <QList>

#include "MatchResultList.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "Team.h"
#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"

namespace QTournament
{


MatchResultList::MatchResultList(TournamentDB* _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat(_cat), round(_round)
{
  // make sure that the requested round is already finished or at least running
  CatRoundStatus crs = cat.getRoundStatus();
  QList<int> runningRounds = crs.getCurrentlyRunningRoundNumbers();
  for (int runningRound : runningRounds)
  {
    if (round <= runningRound) return;   // okay, we are at least in one of the currently running rounds
  }

  if (round <= crs.getFinishedRoundsCount()) return; // okay, we're in one of the finished rounds

  throw std::runtime_error("Requested match results report for unfinished round.");
}

//----------------------------------------------------------------------------

upSimpleReport MatchResultList::regenerateReport() const
{
  // collect the numbers of all match groups in this round
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat, round);
  if (mgl.size() > 1)
  {
    std::sort(mgl.begin(), mgl.end(), [](MatchGroup& mg1, MatchGroup& mg2){
      if (mg1.getGroupNumber() < mg2.getGroupNumber()) return true;
      return false;
    });
  }

  // prepare a subheader if we are in KO-rounds
  QString subHeader = QString();
  if ((mgl.size() == 1) && (mgl.at(0).getGroupNumber() > 0))
  {
    subHeader = GuiHelpers::groupNumToLongString(mgl.at(0).getGroupNumber());
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + tr(" -- Results of Round ") + QString::number(round);
  setHeaderAndHeadline(result.get(), repName, subHeader);
  result->addTab(10.0, SimpleReportLib::TAB_LEFT);
  for (int game=0; game < 5; ++game)
  {
    double colonPos = 150 + game*10.0;
    result->addTab(colonPos - 1.0,  SimpleReportLib::TAB_RIGHT);  // first score
    result->addTab(colonPos,  SimpleReportLib::TAB_CENTER);  // colon
    result->addTab(colonPos + 1.0,  SimpleReportLib::TAB_LEFT);  // second score
  }

  // print a warning if the round is incomplete
  CatRoundStatus crs = cat.getRoundStatus();
  if (round > crs.getFinishedRoundsCount())
  {
    result->writeLine(tr("Note: the round is not finished yet; results are incomplete."), "", 1.0);
  }

  // print the results of each match group
  for (MatchGroup mg : mgl)
  {
    int grpNum = mg.getGroupNumber();

    // print a header if we are in round-robin rounds
    if (grpNum > 0)
    {
      printIntermediateHeader(result, GuiHelpers::groupNumToLongString(grpNum));
    }

    // print each finished match
    MatchList ml = mg.getMatches();
    std::sort(ml.begin(), ml.end(), [](Match& m1, Match& m2){
      return (m1.getMatchNumber() < m2.getMatchNumber());
    });
    bool hasAtLeastOneFinishedMatch = false;
    for (Match ma: ml)
    {
      OBJ_STATE maState = ma.getState();
      if (maState != STAT_MA_FINISHED) continue;

      hasAtLeastOneFinishedMatch = true;

      QString txtLine = QString::number(ma.getMatchNumber()) + "\t";
      txtLine += ma.getPlayerPair1().getDisplayName();
      txtLine += "   :   ";
      txtLine += ma.getPlayerPair2().getDisplayName();
      txtLine += "\t";

      QString scoreString = (ma.getScore())->toString();
      scoreString.replace(",", "\t");
      scoreString.replace(":", "\t:\t");
      txtLine += scoreString;

      // do we need to start a new page?
      if (!(result->hasSpaceForAnotherLine(QString())))
      {
        // the new page is automatically created by the following call
        printIntermediateHeader(result, GuiHelpers::groupNumToLongString(grpNum) + tr(" (cont.)"));
      }

      result->writeLine(txtLine);
    }
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList MatchResultList::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Results::");
  loc += cat.getName() + "::by round::";
  loc += tr("Round ") + QString::number(round);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

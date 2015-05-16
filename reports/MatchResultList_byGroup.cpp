#include <QList>

#include "MatchResultList_byGroup.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "Team.h"
#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "MatchGroup.h"

namespace QTournament
{


MatchResultList_ByGroup::MatchResultList_ByGroup(TournamentDB* _db, const QString& _name, const Category& _cat, int _grpNum)
  :AbstractReport(_db, _name), cat(_cat), grpNum(_grpNum)
{
  // make sure that the requested group is a round-robin group
  // and the group is actually existing
  if (grpNum < 1)
  {
    throw std::runtime_error("Requested match results report for invalid group.");
  }
  for (MatchGroup mg : Tournament::getMatchMngr()->getMatchGroupsForCat(cat, 1))
  {
    if (mg.getGroupNumber() == grpNum) return;  // okay, the round-robin group number exists at least in the first round
  }

  throw std::runtime_error("Requested match results report for invalid group.");
}

//----------------------------------------------------------------------------

upSimpleReport MatchResultList_ByGroup::regenerateReport() const
{
  // collect the match groups with the requested match group number and
  // search in all rounds
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat);
  MatchGroupList filteredList;
  for (MatchGroup mg: mgl)
  {
    if (mg.getGroupNumber() == grpNum) filteredList.append(mg);
  }

  // sort match groups by round number
  if (filteredList.size() > 1)
  {
    std::sort(filteredList.begin(), filteredList.end(), [](MatchGroup& mg1, MatchGroup& mg2){
      if (mg1.getRound() < mg2.getRound()) return true;
      return false;
    });
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + tr(" -- Results of Group ") + QString::number(grpNum);
  setHeaderAndHeadline(result.get(), repName);

  // prepare a tabset for a table with match results
  SimpleReportLib::TabSet ts;
  ts.addTab(8.0, SimpleReportLib::TAB_JUSTIFICATION::TAB_RIGHT);   // the match number
  ts.addTab(12.0, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // the players
  ts.addTab(145, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // dummy tab for a column label
  for (int game=0; game < 5; ++game)
  {
    double colonPos = 150 + game*12.0;
    ts.addTab(colonPos - 1.0,  SimpleReportLib::TAB_JUSTIFICATION::TAB_RIGHT);  // first score
    ts.addTab(colonPos,  SimpleReportLib::TAB_JUSTIFICATION::TAB_CENTER);  // colon
    ts.addTab(colonPos + 1.0,  SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);  // second score
  }

  for (MatchGroup mg : filteredList)
  {
    int round = mg.getRound();
    printIntermediateHeader(result, tr("Round ") + QString::number(round));

    SimpleReportLib::TableWriter tw(ts);
    tw.setHeader(0, tr("Match"));
    tw.setHeader(2, tr("Player"));
    tw.setHeader(3, tr("Game results"));

    for (Match ma : mg.getMatches())
    {
      if (ma.getState() != STAT_MA_FINISHED) continue;

      QStringList rowContent;
      rowContent << "";  // first column not used
      rowContent << QString::number(ma.getMatchNumber());

      QString pNames = ma.getPlayerPair1().getDisplayName();
      pNames += "   :   ";
      pNames += ma.getPlayerPair2().getDisplayName();
      rowContent << pNames;
      rowContent << "";  // a dummy tab for the "Game results" label

      auto ms = ma.getScore();
      assert(ms != nullptr);
      for (int i=0; i < ms->getNumGames(); ++i)
      {
        auto gs = ms->getGame(i);
        assert(gs != nullptr);

        tuple<int, int> sc = gs->getScore();
        rowContent << QString::number(get<0>(sc));
        rowContent << ":";
        rowContent << QString::number(get<1>(sc));
      }
      tw.appendRow(rowContent);
    }
    tw.setNextPageContinuationCaption(tr("Results of round ") + QString::number(round) + tr(" (cont.)"));
    tw.write(result.get());

    if (mg.getState() != STAT_MG_FINISHED)
    {
      result->skip(1.0);
      result->writeLine(tr("Note: this round is not finished yet; results for this group can be incomplete."));
    }

    result->skip(3.0);
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList MatchResultList_ByGroup::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Results::");
  loc += cat.getName() + "::by group::";
  loc += tr("Group ") + QString::number(grpNum);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

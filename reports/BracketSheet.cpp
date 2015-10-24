#include <tuple>

#include <QList>

#include "BracketSheet.h"
#include "SimpleReportGenerator.h"

#include "ui/GuiHelpers.h"
#include "Match.h"
#include "MatchMngr.h"

namespace QTournament
{


BracketSheet::BracketSheet(TournamentDB* _db, const QString& _name, const Category& _cat)
  :AbstractReport(_db, _name), cat(_cat), tabVis(_db->getTab(TAB_BRACKET_VIS))
{
  // make sure the requested category has bracket visualization data
  if (tabVis.getMatchCountForColumnValue(BV_CAT_REF, cat.getId()) == 0)
  {
    throw std::runtime_error("Requested bracket report for a category without bracket visualization data!");
  }
}

//----------------------------------------------------------------------------

upSimpleReport BracketSheet::regenerateReport() const
{
  upSimpleReport result = createEmptyReport_Landscape();

  setHeaderAndHeadline(result.get(), "Bracket", "To be implemented");


  return result;
}

//----------------------------------------------------------------------------

QStringList BracketSheet::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Brackets::");
  loc += cat.getName();

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

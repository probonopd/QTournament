#include "AbstractReport.h"

namespace QTournament
{
AbstractReport::AbstractReport(TournamentDB* _db, const QString& _name)
  :db(_db), name(_name)
{
  if (db == nullptr)
  {
    throw std::runtime_error("Got nullptr for database handle");
  }
  if (name.isEmpty() || (name == ""))
  {
    throw std::runtime_error("Got empty name for report");
  }
}

//----------------------------------------------------------------------------

AbstractReport::~AbstractReport()
{

}

//----------------------------------------------------------------------------

upSimpleReport AbstractReport::createEmptyReport_Portrait()
{
  SimpleReportLib::SimpleReportGenerator* result = new SimpleReportLib::SimpleReportGenerator(
        A4_WIDTH__MM, A4_HEIGHT__MM, DEFAULT_MARGIN__MM);
  result->startNextPage();

  return unique_ptr<SimpleReportLib::SimpleReportGenerator>(result);
}

//----------------------------------------------------------------------------

QString AbstractReport::getName() const
{
  return name;
}

//----------------------------------------------------------------------------

void AbstractReport::setHeaderAndHeadline(SimpleReportLib::SimpleReportGenerator* rep, const QString& headline) const
{
  SimpleReportLib::TabSet ts;
  ts.addTab(A4_WIDTH__MM / 2.0 - DEFAULT_MARGIN__MM, SimpleReportLib::TAB_JUSTIFICATION::TAB_CENTER);
  rep->pushTabs(ts);
  rep->writeLine("\t" + headline, "H1");
  rep->popTabs();
  rep->skip(AFTER_HEADLINE_SKIP__MM);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

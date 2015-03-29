#include "AbstractReport.h"

namespace QTournament
{

  constexpr char AbstractReport::HEADLINE_STYLE[];
  constexpr char AbstractReport::SUBHEADLINE_STYLE[];
  constexpr char AbstractReport::INTERMEDIATEHEADLINE_STYLE[];

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

upSimpleReport AbstractReport::createEmptyReport_Portrait() const
{
  SimpleReportLib::SimpleReportGenerator* rawResult = new SimpleReportLib::SimpleReportGenerator(
        A4_WIDTH__MM, A4_HEIGHT__MM, DEFAULT_MARGIN__MM);
  rawResult->startNextPage();

  upSimpleReport result = upSimpleReport(rawResult);
  prepStyles(result);

  return result;
}

//----------------------------------------------------------------------------

QString AbstractReport::getName() const
{
  return name;
}

//----------------------------------------------------------------------------

void AbstractReport::setHeaderAndHeadline(SimpleReportLib::SimpleReportGenerator* rep, const QString& headline, const QString& subHead) const
{
  SimpleReportLib::TabSet ts;
  ts.addTab(A4_WIDTH__MM / 2.0 - DEFAULT_MARGIN__MM, SimpleReportLib::TAB_JUSTIFICATION::TAB_CENTER);
  rep->pushTabs(ts);
  rep->writeLine("\t" + headline, HEADLINE_STYLE);
  if (subHead.length() > 0)
  {
    rep->skip(HEAD_SUBHEAD_SKIP__MM);
    rep->writeLine("\t" + subHead, SUBHEADLINE_STYLE);
  }
  rep->popTabs();
  rep->skip(AFTER_HEADLINE_SKIP__MM);
}

//----------------------------------------------------------------------------

void AbstractReport::prepStyles(upSimpleReport& rep) const
{
  // basic font: 2 mm Arial
  auto style = rep->getTextStyle();
  style->setFontname(QLatin1Literal("Arial"));
  style->setFontSize_MM(2.0);

  // headlines: 4 mm, bold
  style = rep->createChildTextStyle(HEADLINE_STYLE);
  style->setFontSize_MM(4.0);
  style->setBoldState(true);

  // sub-heaadline: 2,5 mm, italic
  style = rep->createChildTextStyle(SUBHEADLINE_STYLE);
  style->setFontSize_MM(2.5);
  style->setItalicsState(true);

  // intermediate headine: 3,5 mm, not bold
  style = rep->createChildTextStyle(INTERMEDIATEHEADLINE_STYLE);
  style->setFontSize_MM(3.5);

}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

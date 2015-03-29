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

void AbstractReport::printIntermediateHeader(upSimpleReport& rep, const QString& txt, double skipBefore__MM) const
{
  if (skipBefore__MM < 0) skipBefore__MM = 0.0;

  // check for sufficient space
  if (!(rep->hasSpaceForAnotherLine(INTERMEDIATEHEADLINE_STYLE, skipBefore__MM + SKIP_AFTER_INTERMEDIATE_HEADER__MM)))
  {
    rep->startNextPage();
  }
  if (skipBefore__MM > 0) rep->skip(skipBefore__MM);
  rep->writeLine(txt, INTERMEDIATEHEADLINE_STYLE);
  rep->addHorLine();
  rep->skip(SKIP_AFTER_INTERMEDIATE_HEADER__MM);
}

//----------------------------------------------------------------------------

void AbstractReport::prepTabsForMatchResults(upSimpleReport& rep) const
{
  rep->clearAllTabs();

  rep->addTab(10.0, SimpleReportLib::TAB_LEFT);   // Name

  // score tabs for up to five games
  for (int game=0; game < 5; ++game)
  {
    double colonPos = 150 + game*10.0;
    rep->addTab(colonPos - 1.0,  SimpleReportLib::TAB_RIGHT);  // first score
    rep->addTab(colonPos,  SimpleReportLib::TAB_CENTER);  // colon
    rep->addTab(colonPos + 1.0,  SimpleReportLib::TAB_LEFT);  // second score
  }
}

//----------------------------------------------------------------------------

void AbstractReport::printMatchResult(upSimpleReport& rep, const Match& ma, const QString& continuationString) const
{
  OBJ_STATE maState = ma.getState();
  if (maState != STAT_MA_FINISHED) return;

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
  if (!(rep->hasSpaceForAnotherLine(QString())))
  {
    // the new page is automatically created by the following call
    printIntermediateHeader(rep, continuationString);
  }

  rep->writeLine(txtLine);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

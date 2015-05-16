#include <QObject>

#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "AbstractReport.h"

namespace QTournament
{

  constexpr char AbstractReport::HEADLINE_STYLE[];
  constexpr char AbstractReport::SUBHEADLINE_STYLE[];
  constexpr char AbstractReport::INTERMEDIATEHEADLINE_STYLE[];
  constexpr char AbstractReport::RESULTSHEET_NAME_STYLE[];
  constexpr char AbstractReport::RESULTSHEET_TEAM_STYLE[];
  constexpr char AbstractReport::RESULTSHEET_GAMELABEL_STYLE[];

AbstractReport::AbstractReport(TournamentDB* _db, const QString& _name)
  :db(_db), name(_name), cfg(KeyValueTab::getTab(db, TAB_CFG))
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
  rep->writeLine("\t" + headline, HEADLINE_STYLE, 0.0, BEFORE_HEADLINE_SKIP__MM);
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

  // player names in result sheets: 3 mm, bold
  style = rep->createChildTextStyle(RESULTSHEET_NAME_STYLE);
  style->setFontSize_MM(3.0);
  style->setBoldState(true);

  // team names in result sheets: 2 mm, italic
  style = rep->createChildTextStyle(RESULTSHEET_TEAM_STYLE);
  style->setFontSize_MM(2.0);
  style->setItalicsState(true);

  // "Game xx:"-labels in result sheets: 2,5 mm, bold
  style = rep->createChildTextStyle(RESULTSHEET_GAMELABEL_STYLE);
  style->setFontSize_MM(2.5);
  style->setBoldState(true);
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

void AbstractReport::printMatchList(upSimpleReport& rep, const MatchList& maList, const QString& continuationString, bool withResults, bool withGroupColumn) const
{
  // prepare a tabset for a table with match results
  SimpleReportLib::TabSet ts;
  ts.addTab(8.0, SimpleReportLib::TAB_JUSTIFICATION::TAB_RIGHT);   // the match number
  ts.addTab(12.0, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // the players
  ts.addTab(135.0, SimpleReportLib::TAB_JUSTIFICATION::TAB_CENTER);   // the group number
  ts.addTab(145, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // dummy tab for a column label
  for (int game=0; game < 5; ++game)
  {
    double colonPos = 150 + game*12.0;
    ts.addTab(colonPos - 1.0,  SimpleReportLib::TAB_JUSTIFICATION::TAB_RIGHT);  // first score
    ts.addTab(colonPos,  SimpleReportLib::TAB_JUSTIFICATION::TAB_CENTER);  // colon
    ts.addTab(colonPos + 1.0,  SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);  // second score
  }

  // prepare a table for the match results
  SimpleReportLib::TableWriter tw(ts);
  tw.setHeader(0, QObject::tr("Match"));
  tw.setHeader(2, QObject::tr("Players"));
  if (withResults)
  {
    tw.setHeader(4, QObject::tr("Game results"));
  }

  // print the results of all matches
  bool usesRoundRobinGroups = false;
  for (Match ma : maList)
  {
    int grpNum = ma.getMatchGroup().getGroupNumber();
    if (grpNum > 0)
    {
      usesRoundRobinGroups = true;
    }

    QStringList rowContent;
    rowContent << "";  // first column not used
    rowContent << QString::number(ma.getMatchNumber());

    QString pNames = ma.getPlayerPair1().getDisplayName();
    pNames += "   :   ";
    pNames += ma.getPlayerPair2().getDisplayName();
    rowContent << pNames;

    if (withGroupColumn && (grpNum > 0))
    {
      rowContent << QString::number(grpNum);
    } else {
      rowContent << "";
    }

    rowContent << "";  // a dummy tab for the "Game results" label

    if (withResults)
    {
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
    }

    tw.appendRow(rowContent);
  }

  // if used we round robins, add a column header for the group number
  if (usesRoundRobinGroups && withGroupColumn)
  {
    tw.setHeader(3, QObject::tr("Group"));
  }
  tw.setNextPageContinuationCaption(continuationString);
  tw.write(rep.get());
}

//----------------------------------------------------------------------------

void AbstractReport::setHeaderAndFooter(upSimpleReport& rep, const QString& reportName) const
{
  QString tName = cfg[CFG_KEY_TNMT_NAME].toString();
  QString cName = cfg[CFG_KEY_TNMT_ORGA].toString();
  rep->setGlobalHeader(cName, tName, SimpleReportLib::HeaderFooterStrings::TOKEN_CURDATE);

  QString fl = SimpleReportLib::HeaderFooterStrings::TOKEN_CURTIME;

  QString fr = SimpleReportLib::HeaderFooterStrings::TOKEN_CURPGNUM;
  fr += " / ";
  fr += SimpleReportLib::HeaderFooterStrings::TOKEN_TOTALPGNUM;
  rep->setGlobalFooter(fl , reportName, fr);

  rep->applyHeaderAndFooterOnAllPages();
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

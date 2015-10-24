#ifndef BRACKETSHEET_H
#define BRACKETSHEET_H

#include <functional>
#include <tuple>

#include <QObject>

#include "SimpleReportGenerator.h"

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class BracketSheet : public QObject, public AbstractReport
  {
    Q_OBJECT

    enum class BRACKET_TEXT_ELEMENT {
      PAIR1,
      PAIR2,
      INITIAL_RANK,
    };

  public:
    BracketSheet(TournamentDB* _db, const QString& _name, const Category& _cat);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

    static constexpr double GAP_LINE_TXT__MM = 1.0;

  private:
    Category cat;
    DbTab tabVis;

    SimpleReportLib::SimpleReportGenerator* rawReport;  // raw pointer, only to be used during regenerateReport! (BAAAD style)
    double xFac;
    double yFac;

    void determineGridSize();
    tuple<double, double> grid2MM(int gridX, int gridY) const;
    void drawBracketTextItem(int bracketX0, int bracketY0, int ySpan, int orientation, QString txt, BRACKET_TEXT_ELEMENT item);


  };

}
#endif // BRACKETSHEET_H

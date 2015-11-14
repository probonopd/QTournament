#ifndef RESULTSHEETS_H
#define RESULTSHEETS_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class ResultSheets : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    ResultSheets(TournamentDB* _db, const QString& _name, int _numMatches);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  public slots:
    void onMatchSelectionChanged(int newlySelectedMatchId);

  private:
    static constexpr int SHEETS_PER_PAGE = 4;
    static constexpr int GAMES_PER_SHEET = 3;
    static constexpr double SHEET_HEIGHT__MM = 297.0 / SHEETS_PER_PAGE;
    static constexpr double SHEET_TOP_MARGIN__MM = 10.0;

    int firstMatchNum;
    int numMatches;
    void printMatchData(upSimpleReport& rep, const Match& ma) const;
  };

}
#endif // RESULTSHEETS_H

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

    virtual upSimpleReport regenerateReport() const override;
    virtual QStringList getReportLocators() const override;

  public slots:
    void onMatchSelectionChanged(int newlySelectedMatchId);

  private:
    static constexpr int SHEETS_PER_PAGE = 3;
    int firstMatchNum;
    int numMatches;
    void printMatchData(upSimpleReport& rep, const Match& ma) const;
  };

}
#endif // RESULTSHEETS_H

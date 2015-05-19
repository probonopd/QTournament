#ifndef STANDINGS_H
#define STANDINGS_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class Standings : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    Standings(TournamentDB* _db, const QString& _name, const Category& _cat, int _round);

    virtual upSimpleReport regenerateReport() const override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int round;

    int determineBestPossibleRankForPlayerAfterRound(const PlayerPair& pp, int round) const;
    void printBestCaseList(upSimpleReport& rep) const;
  };

}
#endif // STANDINGS_H

#ifndef MARTIXANDSTANDINGS_H
#define MARTIXANDSTANDINGS_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class MartixAndStandings : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    MartixAndStandings(TournamentDB* _db, const QString& _name, const Category& _cat, int _round);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int round;
  };

}
#endif // MARTIXANDSTANDINGS_H

#ifndef INOUTLIST_H
#define INOUTLIST_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class InOutList : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    InOutList(TournamentDB* _db, const QString& _name, const Category& _cat, int _round);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

    static bool isValidCatRoundCombination(const Category& _cat, int _round);

  private:
    Category cat;
    int round;
  };

}
#endif // INOUTLIST_H

#ifndef MATCHRESULTLISTBYGROUP_H
#define MATCHRESULTLISTBYGROUP_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class MatchResultList_ByGroup : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    MatchResultList_ByGroup(TournamentDB* _db, const QString& _name, const Category& _cat, int _grpNum);

    virtual upSimpleReport regenerateReport() const override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int grpNum;
  };

}
#endif // MATCHRESULTLISTBYGROUP_H

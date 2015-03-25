#ifndef PARTICIPANTSLIST_H
#define PARTICIPANTSLIST_H

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class ParticipantsList : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    static constexpr int SORT_BY_NAME = 1;
    static constexpr int SORT_BY_TEAM = 2;

    ParticipantsList(TournamentDB* _db, const QString& _name, int _sortCriterion=SORT_BY_NAME);
    virtual ~ParticipantsList();

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() override;

  private:
    int sortCriterion;
  };

}
#endif // PARTICIPANTSLIST_H

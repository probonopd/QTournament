#ifndef PARTICIPANTSLIST_H
#define PARTICIPANTSLIST_H

#include <functional>

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

    static constexpr double SKIP_BEFORE_NEW_TEAM__MM = 3.0;

    ParticipantsList(TournamentDB* _db, const QString& _name, int _sortCriterion=SORT_BY_NAME);
    virtual ~ParticipantsList();

    virtual upSimpleReport regenerateReport() const override;
    virtual QStringList getReportLocators() const override;

  private:
    int sortCriterion;
    void createNameSortedReport(upSimpleReport& rep) const;
    void createTeamSortedReport(upSimpleReport& rep) const;
    QString getCommaSepCatListForPlayer(const Player& p) const;
  };

}
#endif // PARTICIPANTSLIST_H

#ifndef SCHEDULETABWIDGET_H
#define SCHEDULETABWIDGET_H

#include <QDialog>
#include <QItemSelection>
#include <QItemSelectionModel>

#include "Tournament.h"
#include "Match.h"

namespace Ui {
class ScheduleTabWidget;
}

using namespace QTournament;

class ScheduleTabWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleTabWidget(QWidget *parent = 0);
    ~ScheduleTabWidget();

public slots:
    void onBtnStageClicked();
    void onBtnUnstageClicked();
    void onBtnScheduleClicked();
    void onIdleSelectionChanged(const QItemSelection &, const QItemSelection &);
    void onStagedSelectionChanged(const QItemSelection &, const QItemSelection &);
    void onCourtDoubleClicked(const QModelIndex& index);
    void onRoundCompleted(int catId, int round);
    void onTournamentClosed();
    void onTournamentOpened(Tournament* _tnmt);

private:
    Ui::ScheduleTabWidget *ui;
    void updateButtons();
    void askAndStoreMatchResult(const Match& ma);
    Tournament* tnmt;
};

#endif // SCHEDULETABWIDGET_H

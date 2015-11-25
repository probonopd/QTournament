/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#ifndef DLGSEEDINGEDITOR_H
#define DLGSEEDINGEDITOR_H

#include <QDialog>
#include <QTimer>

#include "PlayerPair.h"
#include "TournamentDB.h"

namespace Ui {
  class DlgSeedingEditor;
}


class DlgSeedingEditor : public QDialog
{
  Q_OBJECT

public:
  explicit DlgSeedingEditor(const QTournament::TournamentDB& _db, QWidget *parent = nullptr);
  ~DlgSeedingEditor();
  void initSeedingList(const QTournament::PlayerPairList& _seed);

public slots:
  void onBtnUpClicked();
  void onBtnDownClicked();
  void onBtnShuffleClicked();
  void onShuffleModeChange();
  void onSelectionChanged();
  QTournament::PlayerPairList getSeeding();

private slots:
  void onKeypressTimerElapsed();

private:
  static constexpr int SUBSEQUENT_KEYPRESS_TIMEOUT__MS = 1000;
  Ui::DlgSeedingEditor *ui;
  std::reference_wrapper<const QTournament::TournamentDB> db;
  void updateButtons();
  bool eventFilter(QObject *target, QEvent *event);
  int positionInput;
  QTimer* keypressTimer;
};

#endif // DLGSEEDINGEDITOR_H

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

#ifndef DLGIMPORTPLAYER_H
#define DLGIMPORTPLAYER_H

#include <QDialog>

#include "ExternalPlayerDB.h"

namespace Ui {
  class DlgImportPlayer;
}


class DlgImportPlayer : public QDialog
{
  Q_OBJECT

public:
  explicit DlgImportPlayer(QWidget *parent = nullptr, QTournament::ExternalPlayerDB* _extDb = nullptr);
  ~DlgImportPlayer() override;

  int getSelectedExternalPlayerId();

public slots:
  virtual int exec() override;

private:
  Ui::DlgImportPlayer *ui;
  QTournament::ExternalPlayerDB* extDb;
  void addPlayerEntryToListWidget(const QTournament::ExternalPlayerDatabaseEntryList& entryList);
  QTournament::ExternalPlayerDatabaseEntryList allDatabaseEntries;
  bool isViewFiltered;

private slots:
  void onNameListSelectionChanged();
  void onSearchStringChanged();
};

#endif // DLGIMPORTPLAYER_H

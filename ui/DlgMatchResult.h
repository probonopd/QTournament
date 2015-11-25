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

#ifndef DLGMATCHRESULT_H
#define DLGMATCHRESULT_H

#include <memory>

#include <QDialog>
#include <QShortcut>

#include "Match.h"
#include "Score.h"

namespace Ui {
  class DlgMatchResult;
}

using namespace QTournament;

class DlgMatchResult : public QDialog
{
  Q_OBJECT

public:
  explicit DlgMatchResult(QWidget *parent, const Match& _ma);
  ~DlgMatchResult();
  unique_ptr<MatchScore> getMatchScore() const;


private slots:
  void onGameScoreSelectionChanged();
  void onRandomResultTriggered();

private:
  Ui::DlgMatchResult *ui;
  Match ma;
  void updateControls();
  bool isGame3Necessary() const;
  bool hasValidResult() const;
  QShortcut* shortcutRandomResult;
};

#endif // DLGMATCHRESULT_H

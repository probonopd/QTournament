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

#ifndef REPORTSTABWIDGET_H
#define REPORTSTABWIDGET_H

#include <memory>
#include <vector>

#include <QWidget>
#include <QTreeWidgetItem>

#include "reports/ReportFactory.h"
#include "reports/AbstractReport.h"
#include "TournamentDB.h"

namespace Ui {
  class ReportsTabWidget;
}

namespace QTournament {
  class Tournament;
}


class ReportsTabWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ReportsTabWidget(QWidget *parent = nullptr);
  ~ReportsTabWidget();

  void updateRepPool();
  void setDatabase(const QTournament::TournamentDB* _db);

public slots:
  void onTreeSelectionChanged();
  void onReloadRequested();
  void onResetRequested();

private:
  const QTournament::TournamentDB* db{nullptr};
  Ui::ReportsTabWidget *ui;
  std::vector<QTournament::upAbstractReport> repPool;
  QTreeWidgetItem* treeRoot{nullptr};

  QTreeWidgetItem* createOrRetrieveTreeItem(const QString& locator);
  QTreeWidgetItem* findTreeItemChildByName(QTreeWidgetItem* _parent, const QString& childName) const;
  void createRootItem();
  void showReport(const QString& repName);
  QTournament::upSimpleReport curReport;
  bool isInResetProcedure{false};
};

#endif // REPORTSTABWIDGET_H

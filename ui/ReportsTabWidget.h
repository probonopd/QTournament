#ifndef REPORTSTABWIDGET_H
#define REPORTSTABWIDGET_H

#include <memory>
#include <vector>

#include <QWidget>
#include <QTreeWidgetItem>

#include "Tournament.h"
#include "reports/ReportFactory.h"
#include "reports/AbstractReport.h"
#include "TournamentDB.h"

namespace Ui {
  class ReportsTabWidget;
}

class ReportsTabWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ReportsTabWidget(QWidget *parent = 0);
  ~ReportsTabWidget();

  void updateRepPool();

public slots:
  void onTournamentOpened(Tournament* tnmt);
  void onTournamentClosed();
  void onTreeSelectionChanged();
  void onBtnReloadClicked();

private:
  Ui::ReportsTabWidget *ui;
  std::vector<upAbstractReport> repPool;
  QTreeWidgetItem* treeRoot;

  QTreeWidgetItem* createOrRetrieveTreeItem(const QString& locator);
  QTreeWidgetItem* findTreeItemChildByName(QTreeWidgetItem* _parent, const QString& childName) const;
  void createRootItem();
  void showReport(const QString& repName);
  upSimpleReport curReport;
};

#endif // REPORTSTABWIDGET_H

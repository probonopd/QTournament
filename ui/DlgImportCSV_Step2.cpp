#include <QMessageBox>

#include <QLabel>
#include <QColor>
#include <QInputDialog>
#include <QHeaderView>
#include <QPushButton>

#include "DlgImportCSV_Step2.h"
#include "ui_DlgImportCSV_Step2.h"
#include "ui/DlgPickPlayerSex.h"
#include "ui/DlgPickTeam.h"
#include "TeamMngr.h"
#include "PlayerMngr.h"
#include "CatMngr.h"
#include "ui/DlgPickCategory.h"
#include "CSVImporter.h"

using namespace QTournament;

DlgImportCSV_Step2::DlgImportCSV_Step2(QWidget *parent, const TournamentDB& _db, const std::vector<CSVImportRecord>& initialData) :
  QDialog(parent),
  ui(new Ui::DlgImportCSV_Step2), db{_db}
{
  ui->setupUi(this);

  connect(ui->tab, SIGNAL(warnCountChanged(int,int,int)), this, SLOT(onWarnCountChanged(int,int,int)), Qt::DirectConnection);
  ui->tab->setData(db, initialData);

  // connect actions for the add / delete buttons
  connect(ui->btnDelete, SIGNAL(clicked(bool)), ui->tab, SLOT(onBtnDelRowClicked()));
  connect(ui->btnInsert, SIGNAL(clicked(bool)), ui->tab, SLOT(onBtnAddRowClicked()));
}

//----------------------------------------------------------------------------

DlgImportCSV_Step2::~DlgImportCSV_Step2()
{
  delete ui;
}

//----------------------------------------------------------------------------

vector<CSVImportRecord> DlgImportCSV_Step2::getRecords() const
{
  return ui->tab->getRecords();
}

//----------------------------------------------------------------------------

void DlgImportCSV_Step2::onWarnCountChanged(int errCount, int warnCount, int totalRowCount)
{
  QString msg = tr("<b>%3 rows; <font color=\"darkRed\">%1 errors</font>, ");
  msg += tr("<font color=\"#808000\">%2 warnings</font></b>");
  msg = msg.arg(errCount).arg(warnCount).arg(totalRowCount);

  // update the counters
  ui->laCount->setText(msg);

  // enable / disbale the import button
  ui->btnImport->setEnabled((errCount == 0) && (ui->tab->getNumRecords() > 0));

  // also update the error message for the current cell
  int row = ui->tab->currentRow();
  int col = ui->tab->currentColumn();
  if ((row >= 0) && (col >= 0)) ui->laMsg->setText(ui->tab->getErrMsg(row, col));

  // also enable / disable the insertion / deletion buttons
  ui->btnDelete->setEnabled(ui->tab->getNumRecords() > 0);
}

//----------------------------------------------------------------------------

void DlgImportCSV_Step2::onCellSelectionChanged(int row, int col)
{
  ui->laMsg->setText(ui->tab->getErrMsg(row, col));
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

CSVDataTableWidget::CSVDataTableWidget(QWidget* parent)
  :GuiHelpers::AutoSizingTableWidget{GuiHelpers::AutosizeColumnDescrList{
{tr("Last name"), 6},
{tr("Given name"), 6},
{tr("Sex"), 1},
{tr("Team"), 4},
{tr("Categories"), 4},
                                     }, parent}, db{nullptr}
{
  setRubberBandCol(4);

  // switch the selection behaviour from "row" (set by the
  // AutoSizingTable) back to "item"
  setSelectionBehavior(QAbstractItemView::SelectItems);

  // re-enable row numbers (have been disabled by the AutoSizingTable)
  verticalHeader()->show();

  // handler for double-clicks
  connect(this, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onCellDoubleClicked(int,int)));
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::setData(const TournamentDB& _db, const std::vector<CSVImportRecord>& initialData)
{
  db = _db;
  records = initialData;

  // prepare a list of categories than still
  // accept new players
  CatMngr cm{db};
  for (const Category& cat : cm.getAllCategories())
  {
    if (cat.canAddPlayers()) availCategories.push_back(cat);
  }
  std::sort(availCategories.begin(), availCategories.end(), [](const Category& c1, const Category& c2)
  {
    return (c1.getName() < c2.getName());
  });

  // also store a list of category names
  for (const Category& cat : availCategories)
  {
    availCatNames.push_back(string{cat.getName().toUtf8().constData()});
  }

  rebuildContents();
  updateWarnings();
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::rebuildContents()
{
  clearContents();
  setRowCount(0);

  for (int row = 0; row < records.size(); ++row)
  {
    insertRow(row);

    // add text contents
    createOrUpdateCellItem(row);
  }
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::updateWarnings()
{
  errList = analyseCSV(db, records);
  int cntFatal = 0;

  // reset all warnings and errors
  for (int row = 0; row < rowCount(); ++row)
  {
    for (int col = 0; col < 5; ++col)
    {
      QTableWidgetItem* i = item(row, col);
      if (i == nullptr) continue;
      i->setData(Qt::UserRole, 0);
      i->setData(Qt::UserRole + 1, false);
      i->setBackgroundColor(Qt::white);
      i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
  }

  // apply the new colors and warning
  for (const CSVError& err : errList)
  {
    if (err.isFatal) ++cntFatal;

    QTableWidgetItem* i = item(err.row, err.column);
    if (i == nullptr) continue;

    // do not accidently overwrite an "error" with a warning
    int lvl = err.isFatal ? 2 : 1;
    if (i->data(Qt::UserRole) > lvl) continue;

    // set the rigth background color and store
    // the warning level
    i->setBackground((lvl == 1) ? Qt::yellow : Qt::red);
    i->setData(Qt::UserRole, lvl);

    // if we have a "name exists"-warning, prevent further
    // modifications of the player's sex
    if (err.err == CSVErrCode::NameNotUnique)
    {
      QTableWidgetItem* i = item(err.row, 2);
      if (i == nullptr) continue;
      i->setData(Qt::UserRole + 1, true);
      i->setFlags(0);
    }
  }

  emit warnCountChanged(cntFatal, errList.size() - cntFatal, records.size());
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::createOrUpdateCellItem(int row, int col)
{
  const CSVImportRecord& rec = records[row];

  QString txt;
  if (col == CSVFieldsIndex::LastName)
  {
    txt = rec.getLastName();
  }
  if (col == CSVFieldsIndex::FirstName)
  {
    txt = rec.getFirstName();
  }
  if (col == CSVFieldsIndex::Sex)
  {
    if (rec.getSex() != DONT_CARE)
    {
      txt = (rec.getSex() == M) ? "m" : tr("f");
    }
  }
  if (col == CSVFieldsIndex::Team)
  {
    txt = rec.getTeamName();
  }
  if (col == CSVFieldsIndex::Categories)
  {
    txt = rec.getCatNames_str();
  }

  createOrUpdateCellItem(row, col, txt);
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::createOrUpdateCellItem(int row)
{
  for (int col = 0; col < 5; ++col) createOrUpdateCellItem(row, col);
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::createOrUpdateCellItem(int row, int col, const QString& txt)
{
  QTableWidgetItem* it = item(row, col);
  if (it == nullptr)
  {
    it = new QTableWidgetItem(txt);
    it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    it->setData(Qt::UserRole, 0);  // 0 = no warnings, 1 = warning, 2 = error
    it->setData(Qt::UserRole + 1, false);  // lock state
    setItem(row, col, it);
  } else {
    it->setText(txt);
  }
}

//----------------------------------------------------------------------------

QString CSVDataTableWidget::getErrMsg(int row, int col)
{
  // check if there are error messages or warnings for the
  // currently selected cell
  QString msg;
  for (const CSVError& err : errList)
  {
    if ((err.row != row) || (err.column != col)) continue;

    QString m;
    switch (err.err)
    {
    case CSVErrCode::CategoryLocked:
      m = tr("You cannot add anymore players to category %1.");
      m = m.arg(err.para);
      break;

    case CSVErrCode::CategoryNotExisting:
      m = tr("The category %1 does not exist and will be ignored during the import.");
      m = m.arg(err.para);
      break;

    case CSVErrCode::CategoryNotSuitable:
      m = tr("The category %1 is not suitable for this player.");
      m = m.arg(err.para);
      break;

    case CSVErrCode::NameNotUnique:
      m = tr("A player of this name already exists in the tournament. If necessary, the team assignment ");
      m += tr("will be updated and the category selection will be merged ");
      m += tr("with the already selected categories.");
      break;

    case CSVErrCode::NameRedundant:
      m = tr("This player already occurs in row %1. ");
      m += tr("Please remove or rename the player in this row.");
      m = m.arg(err.para);
      break;

    case CSVErrCode::NoFirstName:
      m = tr("You must enter a given name for the player.");
      break;

    case CSVErrCode::NoLastName:
      m = tr("You must enter a family name for the player.");
      break;

    case CSVErrCode::NoSex:
      m = tr("You must provide the player's sex.");
      break;

    case CSVErrCode::NoTeamName:
      m = tr("You must provide a team name for the player");
      break;
    }

    if (!(m.isEmpty())) msg += m + "\n\n";
  }

  if (msg.isEmpty()) msg = "--";

  return msg;
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::onCellDoubleClicked(int row, int col)
{
  QString newVal;
  bool hasUpdate = false;

  QTableWidgetItem* curItem = currentItem();
  if (curItem == nullptr) return;

  // is the item locked for editing?
  if (curItem->data(Qt::UserRole + 1).toBool())
  {
    QMessageBox::critical(this, tr("Edit cell"),
                          tr("The name and sex of existing players cannot be changed here."));
    return;
  }

  CSVImportRecord& rec = records[row];

  // enter a new first name or last name
  if (col <= CSVFieldsIndex::FirstName)
  {
    // get the current value
    QString curName;
    if (curItem != nullptr)
    {
      curName = curItem->text();
    }

    newVal = QInputDialog::getText(this, tr("Edit name"), tr("Enter name:"),
                                            QLineEdit::Normal, curName, &hasUpdate);
    if (!hasUpdate) return;  // canceled by user

    if (newVal.isEmpty())
    {
      QMessageBox::critical(this, tr("Edit cell"), tr("The new name may not be empty!"));
      return;
    }

    if (col == CSVFieldsIndex::LastName) hasUpdate = rec.updateLastName(newVal);
    else hasUpdate = rec.updateFirstName(newVal);
  }

  // change the player's sex
  if (col == CSVFieldsIndex::Sex)
  {
    DlgPickPlayerSex dlg{this, ""};
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return;

    newVal = (dlg.getSelectedSex() == M) ? "m" : tr("f");
    hasUpdate = rec.updateSex(dlg.getSelectedSex());
  }

  // change the player's team
  if (col == CSVFieldsIndex::Team)
  {
    DlgPickTeam dlg{this, db};
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return;

    int teamId = dlg.getSelectedTeamId();
    if (teamId < 0) return;  // should not happen

    TeamMngr tm{db};
    Team t = tm.getTeamById(teamId);
    newVal = t.getName();
    hasUpdate = rec.updateTeamName(newVal);
  }

  // change the category selection
  if (col == CSVFieldsIndex::Categories)
  {
    DlgPickCategory dlg{this, db, rec.getSex()};
    QString catList = rec.getCatNames_str();
    dlg.applyPreselection(catList);

    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return;

    newVal = dlg.getSelection_CommaSep();
    hasUpdate = rec.updateCategories(dlg.getSelection_strVec());
  }

  // write the changes
  if (hasUpdate)
  {
    createOrUpdateCellItem(row, col, newVal);

    // if we modified a name so that it
    // matches an existing name, we have to
    // make sure that everything is in sync
    if (col <= CSVFieldsIndex::Sex) createOrUpdateCellItem(row, CSVFieldsIndex::Sex);

    updateWarnings();
  }
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::onBtnDelRowClicked()
{
  int row = currentRow();
  if ((row < 0) || (row >= records.size())) return;

  removeRow(row);
  auto it = records.begin() + row;
  records.erase(it);

  updateWarnings();
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::onBtnAddRowClicked()
{
  string l{tr("New").toUtf8().constData()};
  string f{tr("Player").toUtf8().constData()};
  CSVImportRecord newRec{db, {l, f, "", "", ""}};

  if (records.size() == 0)
  {
    insertRow(0);
    records.push_back(newRec);
    createOrUpdateCellItem(0);
  } else {
    int row = currentRow();
    if ((row < 0) || (row >= records.size())) return;

    // insert the new row after the current row
    ++row;
    insertRow(row);
    if (row == records.size())
    {
      records.push_back(newRec);
    } else {
      auto it = records.begin() + row;
      records.insert(it, newRec);
    }
    createOrUpdateCellItem(row);
  }

  updateWarnings();
}


#include <QMessageBox>

#include <QLabel>
#include <QColor>
#include <QInputDialog>

#include "DlgImportCSV_Step2.h"
#include "ui_DlgImportCSV_Step2.h"
#include "ui/DlgPickPlayerSex.h"
#include "ui/DlgPickTeam.h"
#include "TeamMngr.h"
#include "PlayerMngr.h"
#include "CatMngr.h"
#include "ui/DlgPickCategory.h"

using namespace QTournament;

DlgImportCSV_Step2::DlgImportCSV_Step2(QWidget *parent, TournamentDB* _db, const vector<vector<string>>& initialData) :
  QDialog(parent),
  ui(new Ui::DlgImportCSV_Step2), db{_db}
{
  ui->setupUi(this);

  connect(ui->tab, SIGNAL(warnCountChanged(int,int,int)), this, SLOT(onWarnCountChanged(int,int,int)), Qt::DirectConnection);
  ui->tab->setData(db, initialData);
}

//----------------------------------------------------------------------------

DlgImportCSV_Step2::~DlgImportCSV_Step2()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgImportCSV_Step2::onWarnCountChanged(int errCount, int warnCount, int totalRowCount)
{
  QString msg = tr("<b>%3 rows; <font color=\"darkRed\">%1 errors</font>, ");
  msg += tr("<font color=\"#808000\">%2 warnings</font></b>");
  msg = msg.arg(errCount).arg(warnCount).arg(totalRowCount);

  ui->laCount->setText(msg);

  ui->btnImport->setEnabled(errCount == 0);
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
  setSelectionBehavior(QAbstractItemView::SelectItems);
  connect(this, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onCellDoubleClicked(int,int)));
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::setData(QTournament::TournamentDB* _db, const vector<vector<string> >& _splitData)
{
  db = _db;
  splitData = _splitData;

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

  insertDataOfExistingPlayers();

  rebuildContents();
  updateWarnings();
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::rebuildContents()
{
  clearContents();
  setRowCount(0);

  int row = 0;
  for (const vector<string>& fields : splitData)
  {
    insertRow(row);

    // add text contents
    int col = 0;
    for (const string& s : fields)
    {
      syncedCellUpdate(row, col, s);
      ++col;
    }

    // append empty items for missing
    // columns. We need them to apply proper
    // background colors
    while (col < 5)
    {
      syncedCellUpdate(row, col, QString{});
      ++col;
    }

    ++row;
  }
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::updateWarnings()
{
  errList = analyseCSV(db, splitData);
  int cntFatal = 0;

  // reset all warnings and errors
  for (int row = 0; row < rowCount(); ++row)
  {
    for (int col = 0; col < 5; ++col)
    {
      QTableWidgetItem* i = item(row, col);
      if (i == nullptr) continue;
      i->setData(Qt::UserRole, 0);
      i->setBackgroundColor(Qt::white);
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
    }
  }

  emit warnCountChanged(cntFatal, errList.size() - cntFatal, splitData.size());
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::syncedCellUpdate(int row, int col, const string& txt)
{
  QString _txt = QString::fromUtf8(txt.c_str());
  syncedCellUpdate(row, col, _txt);
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::syncedCellUpdate(int row, int col, const QString& txt)
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

  splitData[row][col] = string{txt.toUtf8().constData()};
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::insertDataOfExistingPlayers()
{
  PlayerMngr pm{db};

  enforceConsistentSex();

  for (vector<string>& fields : splitData)
  {
    QString last = QString::fromUtf8(fields[0].c_str());
    QString first = QString::fromUtf8(fields[1].c_str());

    if (!(last.isEmpty()) && !(first.isEmpty()))
    {
      if (!(pm.hasPlayer(first, last))) continue;

      Player p = pm.getPlayer(first, last);

      if (fields[3].empty())
      {
        Team t = p.getTeam();
        fields[3] = string{t.getName().toUtf8().constData()};
      }

      vector<string> catNames;
      vector<string> alreadyAssignedCats;
      for (const Category& cat : availCategories)
      {
        if (cat.hasPlayer(p))
        {
          string cn = string{cat.getName().toUtf8().constData()};
          alreadyAssignedCats.push_back(cn);
        }
      }
      if (!(fields[4].empty()))
      {
        Sloppy::stringSplitter(catNames, fields[4], ",", true);

        for (const string& cn : alreadyAssignedCats)
        {
          if (Sloppy::isInVector<string>(catNames, cn)) continue;
          catNames.push_back(cn);
        }
      } else {
        catNames = alreadyAssignedCats;
      }
      fields[4] = Sloppy::commaSepStringFromStringList(catNames, ", ");
    }
  }
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::enforceConsistentSex(int specificRow)
{
  PlayerMngr pm{db};

  int minRow = 0;
  int maxRow = splitData.size() - 1;
  if (specificRow >= 0)
  {
    minRow = specificRow;
    maxRow = specificRow;
  }

  for (int row = minRow; row <= maxRow; ++row)
  {
    vector<string>& fields = splitData[row];

    QString last = QString::fromUtf8(fields[0].c_str());
    QString first = QString::fromUtf8(fields[1].c_str());

    if (!(last.isEmpty()) && !(first.isEmpty()))
    {
      if (!(pm.hasPlayer(first, last))) continue;

      Player p = pm.getPlayer(first, last);

      // overwrite whatever sex has been provided by
      // the user
      QString s = (p.getSex() == M) ? "m" : tr("f");
      syncedCellUpdate(row, 2, s);
    }
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
    QString para = QString::fromUtf8(err.para.c_str());
    switch (err.err)
    {
    case CSVErrCode::CategoryLocked:
      m = tr("You cannot add anymore players to category %1.");
      m = m.arg(para);
      break;

    case CSVErrCode::CategoryNotExisting:
      m = tr("The category %1 does not exist and will be ignored during the import.");
      m = m.arg(para);
      break;

    case CSVErrCode::CategoryNotSuitable:
      m = tr("The category %1 is not suitable for this player.");
      m = m.arg(para);
      break;

    case CSVErrCode::InvalidSexIndicator:
      m = tr("Please use only 'm' or 'f' here.");
      break;

    case CSVErrCode::NameNotUnique:
      m = tr("A player of this name already exists. If necessary, the team assignment ");
      m += tr("will be updated and the category selection will be merged ");
      m += tr("with the already selected categories.");
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

    if (!(m.isEmpty())) msg += m + "\n";
  }

  if (msg.isEmpty()) msg = "--";

  return msg;
}

//----------------------------------------------------------------------------

void CSVDataTableWidget::onCellDoubleClicked(int row, int col)
{
  string newVal;
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

  // enter a new first name or last name
  if (col < 2)
  {
    // get the current value
    QString curName;
    if (curItem != nullptr)
    {
      curName = curItem->text();
    }

    QString newName = QInputDialog::getText(this, tr("Edit name"), tr("Enter name:"),
                                            QLineEdit::Normal, curName, &hasUpdate);
    if (newName.isEmpty())
    {
      QMessageBox::critical(this, tr("Edit cell"), tr("The new name may not be empty!"));
      return;
    }

    newVal = string{newName.toUtf8().constData()};

    // make sure that the modification does not result in
    // an existing player name
    QString first;
    QString last;
    if (col == 0)
    {
      last = newName;
      first = QString::fromUtf8(splitData[row][1].c_str());
    } else {
      first = newName;
      last = QString::fromUtf8(splitData[row][0].c_str());
    }
  }

  // change the player's sex
  if (col == 2)
  {
    DlgPickPlayerSex dlg{this, ""};
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return;

    QString newSex = (dlg.getSelectedSex() == M) ? "m" : tr("f");
    newVal = newSex.toUtf8().constData();
    hasUpdate = true;
  }

  // change the player's team
  if (col == 3)
  {
    DlgPickTeam dlg{this, db};
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return;

    int teamId = dlg.getSelectedTeamId();
    if (teamId < 0) return;  // should not happen

    TeamMngr tm{db};
    Team t = tm.getTeamById(teamId);
    newVal = string{t.getName().toUtf8().constData()};
    hasUpdate = true;
  }

  // change the category selection
  if (col == 4)
  {
    DlgPickCategory dlg{this, db};
    QString catList = QString::fromUtf8(splitData[row][4].c_str());
    dlg.applyPreselection(catList);

    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return;

    newVal = string{dlg.getSelection_CommaSep().toUtf8().constData()};
    hasUpdate = true;
  }

  // write the changes
  if (hasUpdate)
  {
    splitData[row][col] = newVal;
    syncedCellUpdate(row, col, newVal);

    // if we modified a name so that it
    // matches an existing name, we have to
    // make sure that everything is in sync
    if (col < 3) enforceConsistentSex(row);

    updateWarnings();
  }
}

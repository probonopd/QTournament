#include <algorithm>

#include <QComboBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

#include "DlgPickTeam.h"
#include "ui_DlgPickTeam.h"
#include "TeamMngr.h"

using namespace QTournament;

DlgPickTeam::DlgPickTeam(QWidget *parent, QTournament::TournamentDB* _db) :
  QDialog(parent),
  ui(new Ui::DlgPickTeam), db{_db}, selectedTeamId{-1}, leShowsHelpText{true}
{
  ui->setupUi(this);

  // fill the combobox with team names
  ui->cbTeam->addItem(tr("<Please select>"), -1);
  TeamMngr tm{db};

  auto allTeams = tm.getAllTeams();
  std::sort(allTeams.begin(), allTeams.end(), [](const Team& t1, const Team& t2)
  {
    return (t1.getName() < t2.getName());
  });
  for (const Team& t : allTeams)
  {
    ui->cbTeam->addItem(t.getName(), t.getId());
  }

  ui->leTeamName->setHelpText(tr("<Enter a new team name>"));

  updateControls();
}

//----------------------------------------------------------------------------

DlgPickTeam::~DlgPickTeam()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgPickTeam::updateControls()
{
  bool useExisting = ui->rbExisting->isChecked();
  ui->cbTeam->setVisible(useExisting);
  ui->cbTeam->setEnabled(useExisting);
  ui->leTeamName->setVisible(!useExisting);
  ui->leTeamName->setEnabled(!useExisting);

  ui->btnOkay->setEnabled(canFinish());
}

//----------------------------------------------------------------------------

void DlgPickTeam::onFinishedRequested()
{
  if (!(canFinish())) return;

  bool useExisting = ui->rbExisting->isChecked();
  if (useExisting)
  {
    // the following must yield a valid ID,
    // otherwise canFinish() would have
    // return false
    selectedTeamId = ui->cbTeam->currentData().toInt();
    accept();
    return;
  }

  //
  // try to create the new team name
  //


  // the name is guaranteed to be non-empty
  // otherwise canFinish() would have
  // return false
  QString tName = ui->leTeamName->text();
  tName = tName.trimmed();

  TeamMngr tm{db};
  ERR e = tm.createNewTeam(tName);
  QString msg;
  if (e != ERR::OK)
  {
    if (e == NAME_EXISTS)
    {
      msg = tr("A team of that name already exists!");
    } else {
      msg = tr("The team name is invalid (e.g., too long)!");
    }

    QMessageBox::critical(this, tr("Create new team"), msg);
    return;
  }

  Team t = tm.getTeam(tName);
  selectedTeamId = t.getId();
  accept();
}

//----------------------------------------------------------------------------

bool DlgPickTeam::canFinish() const
{
  bool useExisting = ui->rbExisting->isChecked();

  if (useExisting)
  {
    return (ui->cbTeam->currentData().toInt() > 0);
  } else {
    return (ui->leTeamName->hasValidText() && !(ui->leTeamName->text().isEmpty()));
  }

  return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

LineEditWithHelpText::LineEditWithHelpText(QWidget* parent)
  :QLineEdit(parent), showsHelpText{false}
{
}

void LineEditWithHelpText::setHelpText(const QString& txt)
{
  setText(txt);
  showsHelpText = true;
}

void LineEditWithHelpText::focusInEvent(QFocusEvent* ev)
{
  if (showsHelpText)
  {
    clear();
    showsHelpText = false;
  }
}

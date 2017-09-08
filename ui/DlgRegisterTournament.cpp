#include <QDate>
#include <QLineEdit>
#include <QPushButton>
#include <QCalendarWidget>

#include "DlgRegisterTournament.h"
#include "ui_DlgRegisterTournament.h"

DlgRegisterTournament::DlgRegisterTournament(QWidget *parent, const QString& tnmtName, const QString club) :
  QDialog(parent),
  ui(new Ui::DlgRegisterTournament)
{
  ui->setupUi(this);

  // initialize the tournament name and the club name
  ui->leTnmtName->setText(tnmtName);
  ui->leClub->setText(club);

  // initialize the date widgets:
  // earliest selectable date is today and last day cannot be behind first day
  QDate today = QDate::currentDate();
  ui->cwFirst->setSelectedDate(today);
  ui->cwLast->setSelectedDate(today);
  ui->cwFirst->setMinimumDate(today);
  ui->cwLast->setMinimumDate(today);
}

//----------------------------------------------------------------------------

DlgRegisterTournament::~DlgRegisterTournament()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgRegisterTournament::onBtnOkayClicked()
{
  accept();
}

//----------------------------------------------------------------------------

void DlgRegisterTournament::onFirstDayChanged()
{
  QDate firstDay = ui->cwFirst->selectedDate();
  QDate lastDay = ui->cwLast->selectedDate();

  // if the new first day is behind the last day, shift
  // the last day to the first day
  if (firstDay > lastDay)
  {
    ui->cwLast->setSelectedDate(firstDay);
  }

  ui->cwLast->setMinimumDate(firstDay);
}

//----------------------------------------------------------------------------

void DlgRegisterTournament::onLastDayChanged()
{

}

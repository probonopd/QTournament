#include <QDate>
#include <QLineEdit>
#include <QPushButton>
#include <QCalendarWidget>
#include <QMessageBox>

#include "DlgRegisterTournament.h"
#include "ui_DlgRegisterTournament.h"
#include "OnlineMngr.h"

using namespace QTournament;

DlgRegisterTournament::DlgRegisterTournament(QWidget *parent, const QString& tnmtName, const QString& club) :
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

QTournament::OnlineRegistrationData DlgRegisterTournament::getValidatedRegistrationData() const
{
  QTournament::OnlineRegistrationData result;

  result.tnmtName = ui->leTnmtName->text().trimmed();
  result.club = ui->leClub->text().trimmed();
  result.personName = ui->leName->text().trimmed();
  result.email = ui->leMail->text().trimmed();
  result.firstDay = ui->cwFirst->selectedDate();
  result.lastDay = ui->cwLast->selectedDate();

  return result;
}

//----------------------------------------------------------------------------

void DlgRegisterTournament::onBtnOkayClicked()
{
  // check for empty fields
  QString msg;
  QString s = ui->leTnmtName->text().trimmed();
  if (s.isEmpty())
  {
    msg += tr("<li>The tournament name may not be empty!</li>");
  }
  s = ui->leClub->text().trimmed();
  if (s.isEmpty())
  {
    msg += tr("<li>The club name may not be empty!</li>");
  }
  s = ui->leName->text().trimmed();
  if (s.isEmpty())
  {
    msg += tr("<li>Your name may not be empty!</li>");
  }
  s = ui->leMail->text().trimmed();
  if (s.isEmpty())
  {
    msg += tr("<li>The email address may not be empty!</li>");
  }
  if (!(msg.isEmpty()))
  {
    msg = tr("<b>One or more errors occurred:</b><ul>") + msg + "</ul><br>";
    QMessageBox::critical(this, "Registration form", msg);
    return;
  }

  // validate the email address
  QString email = ui->leMail->text();
  if (!(Sloppy::isValidEmailAddress(email.toUtf8().constData())))
  {
    msg = tr("The email address you've entered is invalid!");
    QMessageBox::critical(this, "Registration form", msg);
    return;
  }

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

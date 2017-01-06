
#include <QLabel>

#include "TournamentDB.h"
#include "Player.h"
#include "PlayerProfile.h"
#include "MatchGroup.h"

#include "DlgPlayerProfile.h"
#include "ui_DlgPlayerProfile.h"


DlgPlayerProfile::DlgPlayerProfile(const Player& _p, QWidget *parent) :
  QDialog(parent), p{_p}, db{p.getDatabaseHandle()}, pp{p},
  ui(new Ui::DlgPlayerProfile)
{
  ui->setupUi(this);
  ui->playerTab->setDatabase(db);
  ui->umpireTab->setDatabase(db);

  fillTables();
  fillLabels();
}

//----------------------------------------------------------------------------

DlgPlayerProfile::~DlgPlayerProfile()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgPlayerProfile::fillLabels()
{
  OBJ_STATE plStat = p.getState();

  // a lambda for converting a QDateTime to
  // a time string
  auto qdt2str = [](const QDateTime& qdt) -> QString
  {
    return qdt.toString("HH:mm");
  };

  // a lambda for returning a duration string
  // for the delta between "now" and a QDateTime
  const time_t now = QDateTime::currentDateTimeUtc().toTime_t();
  auto qdt2durationString = [&now](const QDateTime& qdt) -> QString
  {
    time_t other = qdt.toTime_t();
    int duration = abs(now - other);

    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    QString sDuration = "%1:%2";
    sDuration = sDuration.arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));

    return sDuration;
  };

  //
  // set the title
  //
  QString txt = tr("Information about %1");
  txt = txt.arg(p.getDisplayName_FirstNameFirst());
  ui->laTitle->setText(txt);

  //
  // set the team name
  //
  txt = p.getTeam().getName();
  ui->laTeam->setText(txt);

  //
  // set the status summary
  //
  txt.clear();
  if (plStat == STAT_PL_IDLE)
  {
    txt = tr("Player is idle. ");

    auto ma = pp.getLastPlayedMatch();
    if (ma != nullptr)
    {
      txt += tr("The last match ended %1 ago.");
      txt = txt.arg(qdt2durationString(ma->getFinishTime()));
    } else {
      txt += tr("The player hasn't played any matches yet.");
    }
  }
  unique_ptr<Match> ma;
  if ((plStat == STAT_PL_PLAYING) || (plStat == STAT_PL_REFEREE))
  {
    if (plStat == STAT_PL_PLAYING)
    {
      txt = tr("Player is playing on court %1 for %2 (match %3, %4, Round %5)");
      ma = pp.getCurrentMatch();
    }
    if (plStat == STAT_PL_REFEREE)
    {
      txt = tr("Player is umpire on court %1 for %2 (match %3, %4, Round %5)");
      ma = pp.getCurrentUmpireMatch();
    }

    if (ma != nullptr)
    {
      auto co = ma->getCourt();
      txt = txt.arg(co != nullptr ? QString::number(co->getNumber()) : "??");

      QDateTime sTime = ma->getStartTime();
      txt = txt.arg(sTime.isValid() ? qdt2durationString(sTime) : "??");

      txt = txt.arg(ma->getMatchNumber());
      txt = txt.arg(ma->getCategory().getName());
      txt = txt.arg(ma->getMatchGroup().getRound());
    } else {
      txt = "Waaaaah!!! Database inconsistency!!! Panic!!";
    }
  }
  if (plStat == STAT_PL_WAIT_FOR_REGISTRATION)
  {
    txt = tr("The player has not yet shown up for registration.");
  }
  ui->laStatus->setText(txt);

  //
  // set the next match
  //
  ma = pp.getNextMatch();
  txt.clear();
  if (ma != nullptr)
  {
    txt = tr("#%1: %2 (%3, Round %4)");
    txt = txt.arg(ma->getMatchNumber());
    txt = txt.arg(ma->getDisplayName(tr("Winner"), tr("Loser")));
    txt = txt.arg(ma->getCategory().getName());
    txt = txt.arg(ma->getMatchGroup().getRound());
  } else {
    txt = "--";
  }
  ui->laNextMatch->setText(txt);

  //
  // set the next umpire match
  //
  ma = pp.getNextUmpireMatch();
  txt.clear();
  if (ma != nullptr)
  {
    txt = tr("#%1: %2 (%3, Round %4)");
    txt = txt.arg(ma->getMatchNumber());
    txt = txt.arg(ma->getDisplayName(tr("Winner"), tr("Loser")));
    txt = txt.arg(ma->getCategory().getName());
    txt = txt.arg(ma->getMatchGroup().getRound());
  } else {
    txt = "--";
  }
  ui->laNextUmpireMatch->setText(txt);

  //
  // set the list of categories and partners
  //
  txt.clear();
  QString txtPartner;
  for (const Category& cat : p.getAssignedCategories())
  {
    txt += cat.getName() + ", ";

    try
    {
      Player partner = cat.getPartner(p);
      txtPartner += "%1 (%2, %3), ";
      txtPartner = txtPartner.arg(partner.getDisplayName_FirstNameFirst());
      txtPartner = txtPartner.arg(cat.getName());
      txtPartner = txtPartner.arg(partner.getTeam().getName());
    }
    catch (...) {}
  }
  if (!(txt.isEmpty() > 0))
  {
    txt.chop(2);
  } else {
    txt = "--";
  }
  if (!(txtPartner.isEmpty() > 0))
  {
    txtPartner.chop(2);
  } else {
    txtPartner = "--";
  }
  ui->laCats->setText(txt);
  ui->laPartners->setText(txtPartner);

  //
  // set the total match count
  //
  txt = tr("%1 matches for this player in this tournament");
  txt = txt.arg(ui->playerTab->rowCount());
  ui->laTotalMatches->setText(txt);

  //
  // set the finished matches count
  //
  txt = tr("%1 matches already finished (%2 actually played, %3 walkovers)");
  txt = txt.arg(pp.getFinishCount());
  txt = txt.arg(pp.getActuallyPlayedCount());
  txt = txt.arg(pp.getWalkoverCount());
  ui->laMatchesFinished->setText(txt);

  //
  // set the waiting matches count
  //
  if (plStat == STAT_PL_PLAYING)
  {
    txt = tr("%1 further matches scheduled and 1 match currently running");
    txt = txt.arg(pp.getScheduledAndNotFinishedCount() - 1);
  } else {
    txt = tr("%1 further matches scheduled");
    txt = txt.arg(pp.getScheduledAndNotFinishedCount());
  }
  ui->laMatchesWaiting->setText(txt);

  //
  // set the other matches count
  //
  txt = tr("%1 other, not yet scheduled matches");
  txt = txt.arg(pp.getOthersCount());
  ui->laOtherMatches->setText(txt);

  //
  // set the umpire statistics
  //
  txt = tr("%1 services (%2 finished, %3 running, %4 waiting)");
  txt = txt.arg(pp.getMatchesAsUmpire().length());
  txt = txt.arg(pp.getUmpireFinishedCount());
  if (plStat == STAT_PL_REFEREE)
  {
    txt = txt.arg(1);
    txt = txt.arg(pp.getUmpireScheduledAndNotFinishedCount() - 1);
  } else {
    txt = txt.arg(0);
    txt = txt.arg(pp.getUmpireScheduledAndNotFinishedCount() - 0);
  }
  ui->laUmpireStatistics->setText(txt);
}

//----------------------------------------------------------------------------

void DlgPlayerProfile::fillTables()
{
  ui->umpireTab->appendMatchList(pp.getMatchesAsUmpire());

  // for the matches as player append them one by one
  // and skip the un-scheduled matches first and then
  // append them in a second loop
  //
  // --> the unscheduled matches go the bottom of the table
  auto maList = pp.getMatchesAsPlayer();
  for (const Match& ma : maList)
  {
    if (ma.getMatchNumber() == MATCH_NUM_NOT_ASSIGNED) continue;
    ui->playerTab->appendMatch(ma);
  }
  for (const Match& ma : maList)
  {
    if (ma.getMatchNumber() != MATCH_NUM_NOT_ASSIGNED) continue;
    ui->playerTab->appendMatch(ma);
  }
}

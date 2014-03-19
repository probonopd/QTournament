/* 
 * File:   dlgEditPlayer.h
 * Author: volker
 *
 * Created on March 18, 2014, 8:00 PM
 */

#ifndef _DLGEDITPLAYER_H
#define	_DLGEDITPLAYER_H

#include "ui_dlgEditPlayer.h"

#include "Player.h"

using namespace QTournament;

class DlgEditPlayer : public QDialog
{
  Q_OBJECT
public:
  DlgEditPlayer (Player* _selectedPlayer = 0);
  virtual ~DlgEditPlayer ();
  QString getFirstName();
  QString getLastName();
  bool hasNameChange();
  SEX getSex();
  Team getTeam();
  
private:
  Ui::dlgEditPlayer ui;
  Player* selectedPlayer;
  void initFromPlayerData();
  void initTeamList();
  bool _hasNameChange;

public slots:
  virtual void done (int result);
};

#endif	/* _DLGEDITPLAYER_H */

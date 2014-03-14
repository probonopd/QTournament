/* 
 * File:   Team.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
 */

#include <stdexcept>

#include "Player.h"
#include "Team.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "PlayerMngr.h"
#include "Tournament.h"

namespace QTournament
{

  Player::Player(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_PLAYER, rowId)
  {
  }

//----------------------------------------------------------------------------

  Player::Player(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  QString Player::getDisplayName(int maxLen) const
  {
    QString first = row[PL_FNAME].toString();
    QString last = row[PL_LNAME].toString();
    
    QString fullName = last + ", " + first;
    
    if (maxLen < 1)   // no length restriction
    {
      return fullName;
    }
    
    if (maxLen < 6)  // invalid length restriction
    {
      throw std::invalid_argument("Can't generate display name of less than six characters");
    }
    
    if (fullName.length() <= maxLen)   // sufficient length restriction
    {
      return fullName;
    }
    
    // okay, we have to shorten the name somehow. Preferably, we keep as much
    // of the last name as possible and shorten the first name
    while (fullName.length() > maxLen)
    {
      if (first.length() > 1)
      {
        first = first.left(first.length() - 1);
      } else {
        last = last.left(last.length() - 1);
      }
      
      fullName = last + ", " + first;
    }
    
    return fullName;
    
  }

//----------------------------------------------------------------------------

  ERR Player::rename(const QString& newFirst, const QString& newLast)
  {
    return Tournament::getPlayerMngr()->renamePlayer(*this, newFirst, newLast);
  }

//----------------------------------------------------------------------------

  QString Player::getFirstName() const
  {
    return row[PL_FNAME].toString();
  }

//----------------------------------------------------------------------------

  QString Player::getLastName() const
  {
    return row[PL_LNAME].toString();
  }

//----------------------------------------------------------------------------

  SEX Player::getSex() const
  {
    int sexInt = row[PL_SEX].toInt();
    return static_cast<SEX>(sexInt);
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
/* 
 * File:   Category.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
 */

#include "Category.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatMngr.h"
#include "Tournament.h"

namespace QTournament
{

  Category::Category(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_CATEGORY, rowId)
  {
  }

//----------------------------------------------------------------------------

  Category::Category(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  QString Category::getName()
  {
    return row[GENERIC_NAME_FIELD_NAME].toString();
  }

//----------------------------------------------------------------------------

  ERR Category::rename(const QString& nn)
  {
    QString newName = nn.trimmed();
    
    // Ensure the new name is valid
    if ((newName.isEmpty()) || (newName.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    // make sure the new name doesn't exist yet
    CatMngr* cm = Tournament::getCatMngr();
    if (cm->hasCategory(newName))
    {
      return NAME_EXISTS;
    }
    
    row.update(GENERIC_NAME_FIELD_NAME, newName);
    
    return OK;
  }

//----------------------------------------------------------------------------

  MATCH_SYSTEM Category::getMatchSystem()
  {
    int sysInt = row[CAT_SYS].toInt();
    
    return static_cast<MATCH_SYSTEM>(sysInt);
  }

//----------------------------------------------------------------------------

  MATCH_TYPE Category::getMatchType()
  {
    int typeInt = row[CAT_MATCH_TYPE].toInt();
    
    return static_cast<MATCH_TYPE>(typeInt);
  }

//----------------------------------------------------------------------------

  SEX Category::getSex()
  {
    int sexInt = row[CAT_SEX].toInt();
    
    return static_cast<SEX>(sexInt);
  }

//----------------------------------------------------------------------------

  ERR Category::setMatchSystem(MATCH_SYSTEM s)
  {
    return Tournament::getCatMngr()->setMatchSystem(*this, s);
  }

//----------------------------------------------------------------------------

  ERR Category::setMatchType(MATCH_TYPE t)
  {
    return Tournament::getCatMngr()->setMatchType(*this, t);
  }

//----------------------------------------------------------------------------

  ERR Category::setSex(SEX s)
  {
    return Tournament::getCatMngr()->setSex(*this, s);
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

}
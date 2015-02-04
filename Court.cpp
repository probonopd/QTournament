/* 
 * File:   Team.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
 */

#include <stdexcept>

#include "Court.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "Tournament.h"

namespace QTournament
{

  Court::Court(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_COURT, rowId)
  {
  }

//----------------------------------------------------------------------------

  Court::Court(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  QString Court::getName(int maxLen) const
  {
    QVariant _result = row[GENERIC_NAME_FIELD_NAME].toString();
    if (_result.isNull()) return QString();  // empty name field

    QString result = _result.toString();

    if ((maxLen > 0) && (result.length() > maxLen))
    {
      result = result.left(maxLen);
    }

    return result;
  }

//----------------------------------------------------------------------------

  ERR Court::rename(const QString &newName)
  {
    return Tournament::getCourtMngr()->renameCourt(*this, newName);
  }

//----------------------------------------------------------------------------

  int Court::getNumber() const
  {
    return row[CO_NUMBER].toInt();
  }

//----------------------------------------------------------------------------

  bool Court::isManualAssignmentOnly() const
  {
    return (row[CO_IS_MANUAL_ASSIGNMENT].toInt() == 1);
  }

//----------------------------------------------------------------------------

  void Court::setManualAssignment(bool isManual)
  {
    row.update(CO_IS_MANUAL_ASSIGNMENT, isManual ? 1 : 0);
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

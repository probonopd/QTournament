/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

  unique_ptr<Match> Court::getMatch() const
  {
    return Tournament::getMatchMngr()->getMatchForCourt(*this);
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

}

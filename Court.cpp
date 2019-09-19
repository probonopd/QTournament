/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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
#include "HelperFunc.h"
#include "CourtMngr.h"
#include "MatchMngr.h"

namespace QTournament
{

  Court::Court(const TournamentDB& _db, int& rowId)
    :TournamentDatabaseObject(_db, TabCourt, rowId)
  {
  }

//----------------------------------------------------------------------------

  Court::Court(const TournamentDB& _db, const SqliteOverlay::TabRow& _row)
  :TournamentDatabaseObject(_db, _row)
  {
  }

//----------------------------------------------------------------------------

  QString Court::getName(int maxLen) const
  {
    auto _result = row.getString2(GenericNameFieldName);
    if (!_result) return QString();  // empty name field

    QString result = stdString2QString(*_result);

    if ((maxLen > 0) && (result.length() > maxLen))
    {
      result = result.left(maxLen);
    }

    return result;
  }

//----------------------------------------------------------------------------

  Error Court::rename(const QString &newName)
  {
    CourtMngr cm{db};
    return cm.renameCourt(*this, newName);
  }

//----------------------------------------------------------------------------

  int Court::getNumber() const
  {
    return row.getInt(CO_Number);
  }

//----------------------------------------------------------------------------

  bool Court::isManualAssignmentOnly() const
  {
    return (row.getInt(CO_IsManualAssignment) == 1);
  }

//----------------------------------------------------------------------------

  void Court::setManualAssignment(bool isManual)
  {
    row.update(CO_IsManualAssignment, isManual ? 1 : 0);
  }

//----------------------------------------------------------------------------

  std::optional<Match> Court::getMatch() const
  {
    MatchMngr mm{db};
    return mm.getMatchForCourt(*this);
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

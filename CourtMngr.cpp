/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <SqliteOverlay/ClausesAndQueries.h>

#include "CourtMngr.h"
#include "HelperFunc.h"
#include "CentralSignalEmitter.h"

using namespace SqliteOverlay;

namespace QTournament
{

  CourtMngr::CourtMngr(const TournamentDB& _db)
  : TournamentDatabaseObjectManager(_db, TabCourt)
  {
  }

//----------------------------------------------------------------------------

  CourtOrError CourtMngr::createNewCourt(const int courtNum, const QString& _name)
  {
    QString name = _name.trimmed();
    
    if (name.length() > MaxNameLen)
    {
      return CourtOrError{Error::InvalidName};
    }
    
    if (hasCourt(courtNum))
    {
      return CourtOrError{Error::CourtNumberExists};
    }
    
    // prepare a new table row
    SqliteOverlay::ColumnValueClause cvc;
    cvc.addCol(CO_Number, courtNum);
    cvc.addCol(GenericNameFieldName, QString2StdString(name));
    cvc.addCol(CO_IsManualAssignment, 0);
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::CO_Avail));
    cvc.addCol(GenericSeqnumFieldName, InvalidInitialSequenceNumber);  // will be fixed immediately; this is just for satisfying a not-NULL constraint

    // create the new court row
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateCourt();
    int newId = tab.insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreateCourt(tab.length() - 1); // the new sequence number is always the highest
    
    // create a court object for the new court and return a pointer
    // to this new object
    return CourtOrError(db, newId);
  }

//----------------------------------------------------------------------------

  bool CourtMngr::hasCourt(const int courtNum)
  {
    return (tab.getMatchCountForColumnValue(CO_Number, courtNum) > 0);
  }

//----------------------------------------------------------------------------

  int CourtMngr::getHighestUsedCourtNumber() const
  {
    static const std::string sql{"SELECT max(" + std::string{CO_Number} + ") FROM " + std::string{TabCourt}};

    try
    {
      return db.execScalarQueryIntOrNull(sql).value_or(0);
    }
    catch (NoDataException&) {
      return 0;
    }
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a court identified by its court number
   *
   * @param courtNum is the number of the court
   *
   * @return a unique_ptr to the requested court or nullptr if the court doesn't exits
   */
  std::optional<Court> CourtMngr::getCourt(const int courtNum)
  {
    return getSingleObjectByColumnValue<Court>(CO_Number, courtNum);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all courts
   *
   * @Return QList holding all courts
   */
  std::vector<Court> CourtMngr::getAllCourts()
  {
    return getAllObjects<Court>();
  }

//----------------------------------------------------------------------------

  Error CourtMngr::renameCourt(Court& c, const QString& _newName)
  {
    QString newName = _newName.trimmed();
    
    // Ensure the new name is valid
    if (newName.length() > MaxNameLen)
    {
      return Error::InvalidName;
    }
        
    c.rowRef().update(GenericNameFieldName, QString2StdString(newName));
    
    CentralSignalEmitter::getInstance()->courtRenamed(c);
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a court identified by its sequence number
   *
   * @param seqNum is the sequence number of the court to look up
   *
   * @return a unique_ptr to the requested court or nullptr if the court doesn't exits
   */
  std::optional<Court> CourtMngr::getCourtBySeqNum(int seqNum)
  {
    return getSingleObjectByColumnValue<Court>(GenericSeqnumFieldName, seqNum);
  }


//----------------------------------------------------------------------------

  bool CourtMngr::hasCourtById(int id)
  {
    return (tab.getMatchCountForColumnValue("id", id) != 0);
  }

//----------------------------------------------------------------------------

  std::optional<Court> CourtMngr::getCourtById(int id)
  {
    return getSingleObjectByColumnValue<Court>("id", id);
  }

  //----------------------------------------------------------------------------

  int CourtMngr::getActiveCourtCount()
  {
    int allCourts = tab.length();
    int disabledCourts = tab.getMatchCountForColumnValue(GenericStateFieldName, static_cast<int>(ObjState::CO_Disabled));

    return (allCourts - disabledCourts);
  }

//----------------------------------------------------------------------------

  std::optional<Court> CourtMngr::getNextUnusedCourt(bool includeManual) const
  {
    int reqState = static_cast<int>(ObjState::CO_Avail);
    SqliteOverlay::WhereClause wc;
    wc.addCol(GenericStateFieldName, reqState);

    // further restrict the search criteria if courts for manual
    // match assignment are excluded
    if (!includeManual)
    {
      wc.addCol(CO_IsManualAssignment, 0);
    }

    // always get the court with the lowest number first
    wc.setOrderColumn_Asc(CO_Number);

    return getSingleObjectByWhereClause<Court>(wc);
  }

//----------------------------------------------------------------------------

  bool CourtMngr::acquireCourt(const Court &co)
  {
    if (co.is_NOT_InState(ObjState::CO_Avail))
    {
      return false;
    }

    co.setState(ObjState::CO_Busy);
    CentralSignalEmitter::getInstance()->courtStatusChanged(co.getId(), co.getSeqNum(), ObjState::CO_Avail, ObjState::CO_Busy);
    return true;
  }

//----------------------------------------------------------------------------

  bool CourtMngr::releaseCourt(const Court &co)
  {
    if (co.is_NOT_InState(ObjState::CO_Busy))
    {
      return false;
    }

    // make sure there is no currently running match
    // assigned to this court
    SqliteOverlay::WhereClause wc;
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Running));
    wc.addCol(MA_CourtRef, co.getId());
    DbTab matchTab{db, TabMatch, false};
    if (matchTab.getMatchCountForWhereClause(wc) > 0)
    {
      return false;   // there is at least one running match assigned to this court
    }

    // all fine, we can fall back to AVAIL
    co.setState(ObjState::CO_Avail);
    CentralSignalEmitter::getInstance()->courtStatusChanged(co.getId(), co.getSeqNum(), ObjState::CO_Busy, ObjState::CO_Avail);
    return true;
  }

  //----------------------------------------------------------------------------

  Error CourtMngr::disableCourt(const Court& co)
  {
    ObjState stat = co.getState();

    if (stat == ObjState::CO_Disabled) return Error::OK;   // nothing to do for us

    // prohibit a state change if the court is in use
    if (stat == ObjState::CO_Busy) return Error::CourtBusy;

    // change the court state and emit a change event
    co.setState(ObjState::CO_Disabled);
    CentralSignalEmitter::getInstance()->courtStatusChanged(co.getId(), co.getSeqNum(), stat, ObjState::CO_Disabled);
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error CourtMngr::enableCourt(const Court& co)
  {
    ObjState stat = co.getState();

    if (stat != ObjState::CO_Disabled) return Error::CourtNotDisabled;

    // change the court state and emit a change event
    co.setState(ObjState::CO_Avail);
    CentralSignalEmitter::getInstance()->courtStatusChanged(co.getId(), co.getSeqNum(), ObjState::CO_Disabled, ObjState::CO_Avail);
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error CourtMngr::deleteCourt(const Court& co)
  {
    // check if the court has already been used in the past
    DbTab matchTab{db, TabMatch, false};
    if (matchTab.getMatchCountForColumnValue(MA_CourtRef, co.getId()) > 0)
    {
      return Error::CourtAlreadyUsed;
    }

    // after this check it is safe to delete to court because we won't
    // harm the database integrity

    CentralSignalEmitter* cse =CentralSignalEmitter::getInstance();
    int oldSeqNum = co.getSeqNum();
    cse->beginDeleteCourt(oldSeqNum);
    tab.deleteRowsByColumnValue("id", co.getId());
    fixSeqNumberAfterDelete(tab, oldSeqNum);
    cse->endDeleteCourt();

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::string CourtMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", GenericNameFieldName, CO_Number};

    return db.getSyncStringForTable(TabCourt, cols, rows);
  }

  //----------------------------------------------------------------------------

  CourtOrError CourtMngr::autoSelectNextUnusedCourt(bool includeManual) const
  {
    // find the next free court that is not subject to manual assignment
    auto nextAutoCourt = getNextUnusedCourt(false);
    if (nextAutoCourt)
    {
      return *nextAutoCourt;
    }

    // Damn, no court for automatic assignment available.
    // So let's check for courts with manual assignment, too.
    auto nextManualCourt = getNextUnusedCourt(true);
    if (!nextManualCourt)
    {
      return Error::NoCourtAvail;
    }

    // great, so there is a free court, but it's for
    // manual assigment only. If we were told to include such
    // manual courts, everything is fine
    if (includeManual)
    {
      return *nextManualCourt;
    }

    // indicate to the user that there would be a manual court
    return Error::OnlyManualCourtAvail;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

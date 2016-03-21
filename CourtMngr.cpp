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

#include "ClausesAndQueries.h"

#include "CourtMngr.h"
#include "Tournament.h"
#include "HelperFunc.h"

using namespace SqliteOverlay;

namespace QTournament
{

  CourtMngr::CourtMngr(TournamentDB* _db)
  : TournamentDatabaseObjectManager(_db, TAB_COURT)
  {
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::createNewCourt(const int courtNum, const QString& _name, ERR *err)
  {
    QString name = _name.trimmed();
    
    if (name.length() > MAX_NAME_LEN)
    {
      if (err != nullptr) *err = INVALID_NAME;
      return nullptr;
    }
    
    if (hasCourt(courtNum))
    {
      if (err != nullptr) *err = COURT_NUMBER_EXISTS;
      return nullptr;
    }
    
    // prepare a new table row
    SqliteOverlay::ColumnValueClause cvc;
    cvc.addIntCol(CO_NUMBER, courtNum);
    cvc.addStringCol(GENERIC_NAME_FIELD_NAME, QString2StdString(name));
    cvc.addIntCol(CO_IS_MANUAL_ASSIGNMENT, 0);
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CO_AVAIL));
    
    // create the new court row
    emit beginCreateCourt();
    int newId = tab->insertRow(cvc);
    fixSeqNumberAfterInsert();
    emit endCreateCourt(tab->length() - 1); // the new sequence number is always the highest
    
    // create a court object for the new court and return a pointer
    // to this new object
    Court* co_raw = new Court(db, newId);
    if (err != nullptr) *err = OK;
    return unique_ptr<Court>(co_raw);
  }

//----------------------------------------------------------------------------

  bool CourtMngr::hasCourt(const int courtNum)
  {
    return (tab->getMatchCountForColumnValue(CO_NUMBER, courtNum) > 0);
  }

//----------------------------------------------------------------------------

  int CourtMngr::getHighestUnusedCourtNumber() const
  {
    // do we have any courts at all?
    if (tab->length() == 0) return 1;

    // okay, get the highest used court number
    SqliteOverlay::WhereClause wc;
    wc.addIntCol("id", ">", 0);
    wc.setOrderColumn_Desc(CO_NUMBER);
    auto r = tab->getSingleRowByWhereClause(wc);

    return r.getInt(CO_NUMBER) + 1;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a court identified by its court number
   *
   * @param courtNum is the number of the court
   *
   * @return a unique_ptr to the requested court or nullptr if the court doesn't exits
   */
  unique_ptr<Court> CourtMngr::getCourt(const int courtNum)
  {
    return getSingleObjectByColumnValue<Court>(CO_NUMBER, courtNum);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all courts
   *
   * @Return QList holding all courts
   */
  vector<Court> CourtMngr::getAllCourts()
  {
    return getAllObjects<Court>();
  }

//----------------------------------------------------------------------------

  ERR CourtMngr::renameCourt(Court& c, const QString& _newName)
  {
    QString newName = _newName.trimmed();
    
    // Ensure the new name is valid
    if (newName.length() > MAX_NAME_LEN)
    {
      return INVALID_NAME;
    }
        
    c.row.update(GENERIC_NAME_FIELD_NAME, QString2StdString(newName));
    
    emit courtRenamed(c);
    
    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a court identified by its sequence number
   *
   * @param seqNum is the sequence number of the court to look up
   *
   * @return a unique_ptr to the requested court or nullptr if the court doesn't exits
   */
  unique_ptr<Court> CourtMngr::getCourtBySeqNum(int seqNum)
  {
    return getSingleObjectByColumnValue<Court>(GENERIC_SEQNUM_FIELD_NAME, seqNum);
  }


//----------------------------------------------------------------------------

  bool CourtMngr::hasCourtById(int id)
  {
    return (tab->getMatchCountForColumnValue("id", id) != 0);
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::getCourtById(int id)
  {
    return getSingleObjectByColumnValue<Court>("id", id);
  }

  //----------------------------------------------------------------------------

  int CourtMngr::getActiveCourtCount()
  {
    int allCourts = tab->length();
    int disabledCourts = tab->getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CO_DISABLED));

    return (allCourts - disabledCourts);
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::getNextUnusedCourt(bool includeManual) const
  {
    int reqState = static_cast<int>(STAT_CO_AVAIL);
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, reqState);

    // further restrict the search criteria if courts for manual
    // match assignment are excluded
    if (!includeManual)
    {
      wc.addIntCol(CO_IS_MANUAL_ASSIGNMENT, 0);
    }

    // always get the court with the lowest number first
    wc.setOrderColumn_Asc(CO_NUMBER);

    return getSingleObjectByWhereClause<Court>(wc);
  }

//----------------------------------------------------------------------------

  bool CourtMngr::acquireCourt(const Court &co)
  {
    if (co.getState() != STAT_CO_AVAIL)
    {
      return false;
    }

    co.setState(STAT_CO_BUSY);
    emit courtStatusChanged(co.getId(), co.getSeqNum(), STAT_CO_AVAIL, STAT_CO_BUSY);
    return true;
  }

//----------------------------------------------------------------------------

  bool CourtMngr::releaseCourt(const Court &co)
  {
    if (co.getState() != STAT_CO_BUSY)
    {
      return false;
    }

    // make sure there is no currently running match
    // assigned to this court
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_RUNNING));
    wc.addIntCol(MA_COURT_REF, co.getId());
    auto matchTab = db->getTab(TAB_MATCH);
    if (matchTab->getMatchCountForWhereClause(wc) > 0)
    {
      return false;   // there is at least one running match assigned to this court
    }

    // all fine, we can fall back to AVAIL
    co.setState(STAT_CO_AVAIL);
    emit courtStatusChanged(co.getId(), co.getSeqNum(), STAT_CO_BUSY, STAT_CO_AVAIL);
    return true;
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::autoSelectNextUnusedCourt(ERR *err, bool includeManual) const
  {
    // find the next free court that is not subject to manual assignment
    auto nextAutoCourt = getNextUnusedCourt(false);
    if (nextAutoCourt != nullptr)
    {
      // okay, we have a regular court
      if (err != nullptr) *err = OK;
      return nextAutoCourt;
    }

    // Damn, no court for automatic assignment available.
    // So let's check for courts with manual assignment, too.
    auto nextManualCourt = getNextUnusedCourt(true);
    if (nextManualCourt == nullptr)
    {
      // okay, there is court available at all
      if (err != nullptr) *err = NO_COURT_AVAIL;
      return nullptr;
    }

    // great, so there is a free court, but it's for
    // manual assigment only. If we were told to include such
    // manual courts, everything is fine
    if (includeManual)
    {
      if (err != nullptr) *err = OK;
      return nextManualCourt;
    }

    // indicate to the user that there would be a manual court
    if (err != nullptr) *err = ONLY_MANUAL_COURT_AVAIL;
    return nullptr;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

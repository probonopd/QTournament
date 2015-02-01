/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include <stdexcept>

#include "CourtMngr.h"
#include "Tournament.h"

using namespace dbOverlay;

namespace QTournament
{

  CourtMngr::CourtMngr(TournamentDB* _db)
  : GenericObjectManager(_db), courtTab((*db)[TAB_COURT])
  {
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::createNewCourt(const int courtNum, const QString& _name, ERR *err)
  {
    assert (err != nullptr);

    QString name = _name.trimmed();
    
    if (name.length() > MAX_NAME_LEN)
    {
      *err = INVALID_NAME;
      return nullptr;
    }
    
    if (hasCourt(courtNum))
    {
      *err = COURT_NUMBER_EXISTS;
      return nullptr;
    }
    
    // prepare a new table row
    QVariantList qvl;
    qvl << CO_NUMBER << courtNum;
    qvl << GENERIC_NAME_FIELD_NAME << name;
    qvl << CO_IS_MANUAL_ASSIGNMENT << 0;
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_CO_AVAIL);
    
    // create the new court row
    emit beginCreateCourt();
    int newId = courtTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_COURT);
    emit endCreateCourt(courtTab.length() - 1); // the new sequence number is always the greatest
    
    // create a court object for the new court and return a pointer
    // to this new object
    Court* co_raw = new Court(db, newId);
    *err = OK;
    return unique_ptr<Court>(co_raw);
  }

//----------------------------------------------------------------------------

  bool CourtMngr::hasCourt(const int courtNum)
  {
    return (courtTab.getMatchCountForColumnValue(CO_NUMBER, courtNum) > 0);
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
    if (!(hasCourt(courtNum)))
    {
      return nullptr;
    }
    
    QVariantList qvl;
    qvl << CO_NUMBER << courtNum;
    TabRow r = courtTab.getSingleRowByColumnValue(qvl);
    
    Court* co_raw = new Court(db, r);
    return unique_ptr<Court>(co_raw);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all courts
   *
   * @Return QList holding all courts
   */
  QList<Court> CourtMngr::getAllCourts()
  {
    QList<Court> result;
    
    DbTab::CachingRowIterator it = courtTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << Court(db, *it);
      ++it;
    }
    
    return result;
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
        
    c.row.update(GENERIC_NAME_FIELD_NAME, newName);
    
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
    try {
      TabRow r = courtTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return unique_ptr<Court>(new Court(db, r));
    }
    catch (std::exception e)
    {
    }

    return nullptr;
  }


//----------------------------------------------------------------------------

  bool CourtMngr::hasCourtById(int id)
  {
    try
    {
      TabRow r = courtTab[id];
      return true;
    }
    catch (std::exception e)
    {
    }
    return false;
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::getCourtById(int id)
  {
    // this public function essentially short-circuits the private Court()-constructor... Hmmm...
    try {
      return unique_ptr<Court>(new Court(db, id));
    }
    catch (std::exception e)
    {
    }

    return nullptr;
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> CourtMngr::getNextUnusedCourt(bool includeManual)
  {
    QVariantList qvl;
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_CO_AVAIL);

    // further restrict the search criteria if courts for manual
    // match assignment are excluded
    if (!includeManual)
    {
      qvl << CO_IS_MANUAL_ASSIGNMENT << 0;
    }

    if (courtTab.getMatchCountForColumnValue(qvl) < 1)
    {
      return nullptr;   // no free courts available
    }

    TabRow r = courtTab.getSingleRowByColumnValue(qvl);
    return unique_ptr<Court>(new Court(db, r));
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

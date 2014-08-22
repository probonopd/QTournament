/* 
 * File:   MatchMngr.cpp
 * Author: volker
 * 
 * Created on August 22, 2014, 7:32 PM
 */

#include "MatchMngr.h"

using namespace dbOverlay;

namespace QTournament {

  MatchMngr::MatchMngr(TournamentDB* _db)
  : GenericObjectManager(_db), matchTab((*db)[TAB_MATCH]), groupTab((*db)[TAB_MATCH_GROUP])
  {
  }

//----------------------------------------------------------------------------

  ERR MatchMngr::createMatchGroup(const Category& cat, const int round, const int grpNum, MatchGroup** newGroup)
  {
    // default return value: no group created
    *newGroup = 0;
    
    // check parameters for validity and make sure the match doesn't already exist
    ERR e = getMatchGroup(cat, round, grpNum, newGroup);
    if (e == OK)
    {
      delete *newGroup;
      *newGroup = 0;
      return MATCH_GROUP_EXISTS;
    }
    if (e != NO_SUCH_MATCH_GROUP) return e;

    
    /*
     * Theoretically, we would need more logic checks here. Examples:
     *   - create no match group with a round number that's less than the number of an already finished / running round
     *   - don't mix real (positive) group numbers and special values (negative) in the same round
     *   - don't have e.g. semi finals in an earlier round than quarter finals
     *   - etc.
     * 
     * But for now, let's skip that ;)
     */
    
    // Okay, parameters are valid
    // create a new match group entry in the database
    QVariantList qvl;
    qvl << MG_CAT_REF << cat.getId();
    qvl << MG_ROUND << round;
    qvl << MG_GRP_NUM << grpNum;
    int newId = groupTab.insertRow(qvl);
    
    // create a match group object for the new group an return a pointer
    // to this new object
    *newGroup = new MatchGroup(db, newId);
    
    return OK;
  }

//----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getMatchGroupsForCat(const Category& cat, int round)
  {
    QVariantList qvl;
    qvl << MG_CAT_REF << cat.getId();
    if (round > 0)
    {
      qvl << MG_ROUND << round;
    }
    DbTab::CachingRowIterator it = groupTab.getRowsByColumnValue(qvl);
    
    MatchGroupList result;
    while (!(it.isEnd()))
    {
      result << MatchGroup(db, *it);
      ++it;
    }
    
    return result;
  }
  
//----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getAllMatchGroups()
  {
    MatchGroupList result;
    
    DbTab::CachingRowIterator it = groupTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << MatchGroup(db, *it);
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  ERR MatchMngr::getMatchGroup(const Category& cat, const int round, const int grpNum, MatchGroup** grp)
  {
    // default return value: no group found
    *grp = 0;
    
    // check round parameter for validity
    if (round <= 0) return INVALID_ROUND;
    
    // check round parameter for validity
    if (grpNum <= 0)
    {
      if ((grpNum != GROUP_NUM__FINAL)
	   && (grpNum != GROUP_NUM__SEMIFINAL)
	   && (grpNum != GROUP_NUM__QUARTERFINAL)
	   && (grpNum != GROUP_NUM__L16)
	   && (grpNum != GROUP_NUM__ITERATION)) return INVALID_GROUP_NUM;
    }
    
    QVariantList qvl;
    qvl << MG_CAT_REF << cat.getId();
    qvl << MG_ROUND << round;
    qvl << MG_GRP_NUM << grpNum;
    
    try
    {
      TabRow r = groupTab.getSingleRowByColumnValue(qvl);
      *grp = new MatchGroup(db, r);
      return OK;
    } catch (exception e) {
      return NO_SUCH_MATCH_GROUP;
    }
  }

//----------------------------------------------------------------------------

  bool MatchMngr::hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err)
  {
    MatchGroup* g;
    ERR e = getMatchGroup(cat, round, grpNum, &g);
    
    if (e == OK)
    {
      if (err != 0) *err = OK;
      delete g;
      return true;
    }
    
    if (err != 0) *err = e;
    return false;
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

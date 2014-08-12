/* 
 * File:   GroupDef.cpp
 * Author: volker
 * 
 * Created on August 11, 2014, 7:49 PM
 */

#include "GroupDef.h"

#include <stdexcept>

namespace QTournament {

  GroupDef::GroupDef(int groupSize, int nGroups)
  {
    if (groupSize < 3)
    {
      throw std::invalid_argument("Group size must be greater than two");
    }
    
    if (nGroups < 0)
    {
      throw std::invalid_argument("Number of groups must be positive or zero");
    }
    
    size = groupSize;
    numOfGroups = nGroups;
  }

//----------------------------------------------------------------------------

  // Default constructor only for dynamic memory allocation!!
  GroupDef::GroupDef()
  {
    size = -1;
    numOfGroups = 0;
  }

//----------------------------------------------------------------------------

  GroupDef::GroupDef(const GroupDef& orig)
  {
    copy(orig);
  }

//----------------------------------------------------------------------------

  GroupDef::~GroupDef()
  {
  }

//----------------------------------------------------------------------------

  void GroupDef::copy(const GroupDef& orig)
  {
    size = orig.size;
    numOfGroups = orig.numOfGroups;    
  }

//----------------------------------------------------------------------------

  bool GroupDef::setGroupSize(int newGroupSize)
  {
    if (newGroupSize < 3) return false;
    size = newGroupSize;
    return true;
  }

//----------------------------------------------------------------------------

  bool GroupDef::setNumGroups(int newNumGroups)
  {
    if (newNumGroups < 0) return false;
    numOfGroups = newNumGroups;
    return true;
  }

//----------------------------------------------------------------------------

  int GroupDef::getGroupSize()
  {
    if (size < 0)
    {
      throw std::runtime_error("GroupDef not properly initialized");
    }
    
    return size;
  }

//----------------------------------------------------------------------------

  int GroupDef::getNumGroups()
  {
    if (size < 0)
    {
      throw std::runtime_error("GroupDef not properly initialized");
    }
    
    return numOfGroups;
  }

//----------------------------------------------------------------------------

  int GroupDef::getNumMatches()
  {
    if (size < 0)
    {
      throw std::runtime_error("GroupDef not properly initialized");
    }
    
    // example for four players in a group:
    //
    // #1 plays #2, #3 and #4
    // #2 plays #3 and #4
    // #3 plays #4
    //
    // so we end up with (s-1) + (s-2) + ... + 1
    // with s being the number of players in the group
    //
    // That's basically the sum over all numbers from s to 1, not including s:
    //
    // sum = (s*(s+1)) / 2 - s
    
    int matchesPerGroup = (size * (size+1))/2 - size;
    
    return matchesPerGroup * numOfGroups;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}

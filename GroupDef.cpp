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

#include "GroupDef.h"
#include "TournamentDataDefs.h"

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
    if (newGroupSize > MaxGroupSize) return false;
    size = newGroupSize;
    return true;
  }

//----------------------------------------------------------------------------

  bool GroupDef::setNumGroups(int newNumGroups)
  {
    if (newNumGroups < 0) return false;
    if (newNumGroups > MaxGroupCount) return false;
    numOfGroups = newNumGroups;
    return true;
  }

//----------------------------------------------------------------------------

  int GroupDef::getGroupSize() const
  {
    if (size < 0)
    {
      throw std::runtime_error("GroupDef not properly initialized");
    }
    
    return size;
  }

//----------------------------------------------------------------------------

  int GroupDef::getNumGroups() const
  {
    if (size < 0)
    {
      throw std::runtime_error("GroupDef not properly initialized");
    }
    
    return numOfGroups;
  }

//----------------------------------------------------------------------------

  int GroupDef::getNumMatches() const
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

  int GroupDefList::getTotalGroupCount() const
  {
    int result = 0;
    
    for (int i=0; i < count(); i++)
    {
      result += at(i).getNumGroups();
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  int GroupDefList::getTotalMatchCount() const
  {
    int result = 0;
    
    for (int i=0; i < count(); i++)
    {
      result += at(i).getNumMatches();
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  int GroupDefList::getTotalPlayerCount() const
  {
    int result = 0;
    
    for (int i=0; i < count(); i++)
    {
      result += at(i).getGroupSize() * at(i).getNumGroups();
    }
    
    return result;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}

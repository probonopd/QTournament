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

#ifndef GROUPDEF_H
#define	GROUPDEF_H

#include <QList>

namespace QTournament
{

  class GroupDef
  {
  public:
    GroupDef();
    GroupDef(int groupSize, int nGroups=0);
    GroupDef(const GroupDef& orig);
    virtual ~GroupDef();
    
    bool setGroupSize(int newGroupSize);
    bool setNumGroups(int newNumGroups);
    
    void copy(const GroupDef& orig);
    
    int getGroupSize() const;
    int getNumGroups() const;
    
    int getNumMatches() const;
    
    inline bool operator== (const GroupDef& other) const
    {
      return ((other.size == size) && (other.numOfGroups == numOfGroups));
    }
    
    inline bool operator!= (const GroupDef& other) const
    {
      return (!(this->operator == (other)));
    }
    
  private:
    int size;
    int numOfGroups;
  } ;
  
  class GroupDefList : public QList<GroupDef>
  {
  public:
    GroupDefList() : QList<GroupDef>(){};
    
    int getTotalGroupCount() const;
    int getTotalMatchCount() const;
    int getTotalPlayerCount() const;
  };
}
#endif	/* GROUPDEF_H */


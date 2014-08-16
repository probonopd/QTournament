/* 
 * File:   GroupDef.h
 * Author: volker
 *
 * Created on August 11, 2014, 7:49 PM
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


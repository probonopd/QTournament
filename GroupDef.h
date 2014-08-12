/* 
 * File:   GroupDef.h
 * Author: volker
 *
 * Created on August 11, 2014, 7:49 PM
 */

#ifndef GROUPDEF_H
#define	GROUPDEF_H

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
    
    int getGroupSize();
    int getNumGroups();
    
    int getNumMatches();
    
  private:
    int size;
    int numOfGroups;
  } ;
}
#endif	/* GROUPDEF_H */


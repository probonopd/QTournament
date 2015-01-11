/* 
 * File:   KO_Config.h
 * Author: volker
 *
 * Created on August 11, 2014, 7:40 PM
 */

#ifndef KO_CONFIG_H
#define	KO_CONFIG_H

#include "TournamentDataDefs.h"
#include "GroupDef.h"
#include <QString>

namespace QTournament
{

  //typedef QList<GroupDef> GroupDefList;
  
  class KO_Config
  {
  public:
    KO_Config(KO_START _startLevel, bool _secondSurvives, GroupDefList grps = GroupDefList());
    KO_Config(QString iniString);
    KO_Config(const KO_Config& orig);
    virtual ~KO_Config();
    
    bool isValid(int opponentCount = -1) const;
    int getNumMatches() const;
    int getNumGroupMatches() const;
    QString toString() const;
    KO_START getStartLevel() const;
    bool getSecondSurvives() const;
    int getNumGroupDefs() const;
    GroupDef getGroupDef(int i) const;
    int getNumReqGroups() const;
    GroupDefList getGroupDefList() const;
    int getNumGroups() const;
    
    void setStartLevel(KO_START newLvl);
    void setSecondSurvives(bool newSurvive);
    
  private:
    KO_START startLvl;
    bool secondSurvives;
    GroupDefList grpDefs;
  } ;
}

#endif	/* KO_CONFIG_H */


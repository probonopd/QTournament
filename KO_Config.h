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
#include <QList>

namespace QTournament
{

  class KO_Config
  {
  public:
    KO_Config(KO_START _startLevel, bool _secondSurvives, QList<GroupDef> grps);
    KO_Config(QString iniString);
    KO_Config(const KO_Config& orig);
    virtual ~KO_Config();
    
    bool isValid();
    int getNumMatches();
    QString toString();
    KO_START getStartLevel();
    bool getSecondSurvives();
    int getNumGroups();
    GroupDef getGroupDef(int i);
    
  private:
    KO_START startLvl;
    bool secondSurvives;
    QList<GroupDef> grpDefs;
  } ;
}

#endif	/* KO_CONFIG_H */


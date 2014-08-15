/* 
 * File:   KO_Config.cpp
 * Author: volker
 * 
 * Created on August 11, 2014, 7:40 PM
 */

#include <stdexcept>

#include "KO_Config.h"
#include "TournamentDataDefs.h"
#include <QString>
#include <qt/QtCore/qstringlist.h>

namespace QTournament {

  KO_Config::KO_Config(KO_START _startLevel, bool _secondSurvives, QList<GroupDef> grps)
  {
    startLvl = _startLevel;
    secondSurvives = _secondSurvives;
    
    for (int n=0; n<grps.count(); n++)
    {
      grpDefs.append(grps.at(n));
    }
  }

//----------------------------------------------------------------------------

  KO_Config::KO_Config(const KO_Config& orig)
  {
    startLvl = orig.startLvl;
    secondSurvives = orig.secondSurvives;
    
    for (int n=0; n<orig.grpDefs.count(); n++)
    {
      grpDefs.append(orig.grpDefs.at(n));
    }
  }

//----------------------------------------------------------------------------

  KO_Config::~KO_Config()
  {
  }

//----------------------------------------------------------------------------

  bool KO_Config::isValid()
  {
    // calculate the number of required groups,
    // based on the start configuration of the KO rounds
    int reqGroups = 16;
    if (startLvl == SEMI)
    {
      reqGroups = 4;
    }
    else if (startLvl == QUARTER)
    {
      reqGroups = 8;
    }
    
    // if also the second-placed player of each group
    // qualifies for the KO rounds, we only need half
    // as many groups
    if (secondSurvives)
    {
      reqGroups = reqGroups / 2;
    }
    
    // compare the required number with the actual number of groups
    return reqGroups == grpDefs.count();
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumMatches()
  {
    int n = 2;  // Final plus match for third place
    if (startLvl == SEMI) n += 2;   // two semi-finals
    if (startLvl == QUARTER) n += 2 + 4; // semi-finals plus four quarter finals
    if (startLvl == L16) n += 2 + 4 + 8; // ... plus eight matches of the last 16
    
    for (int i=0; i<grpDefs.count(); i++)
    {
      n += grpDefs[i].getNumMatches();
    }
    
    return n;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumGroups()
  {
    return grpDefs.count();
  }

//----------------------------------------------------------------------------

  bool KO_Config::getSecondSurvives()
  {
    return secondSurvives;
  }

//----------------------------------------------------------------------------

  GroupDef KO_Config::getGroupDef(int i)
  {
    if ((i < 0) || (i >= grpDefs.count()))
    {
      throw std::invalid_argument("Group index not valid!");
    }
    
    return grpDefs.at(i);
  }

//----------------------------------------------------------------------------

  KO_START KO_Config::getStartLevel()
  {
    return startLvl;
  }

//----------------------------------------------------------------------------

  QString KO_Config::toString()
  {
    QString result = "L16";
    
    if (startLvl == SEMI) result = "S";
    else if (startLvl == QUARTER) result = "Q";
    
    result += ";";
    
    if (secondSurvives) result += "1";
    else result += "0";
    
    result += ";";
    
    for (int i=0; i < grpDefs.count(); i++)
    {
      GroupDef g = grpDefs.at(i);
      
      result += QString::number(g.getNumGroups()) + ";";
      result += QString::number(g.getGroupSize()) + ";";
    }
    
    return result;
    
  }

//----------------------------------------------------------------------------

  KO_Config::KO_Config(QString iniString)
  {
    // first consistency check:
    // we must have at least two ';' in the string and
    // the number of ';' must be even
    int nSemicolon = iniString.count(";");
    if ((nSemicolon < 2) || ((nSemicolon % 2) != 0))
    {
      throw std::invalid_argument("Initialization string is invalid (Semicolon count)!");
    }
    
    QStringList fields = iniString.split(";");
    
    // try to parse the first data field (the start level)
    QString lvl = fields.at(0);
    lvl = lvl.trimmed();
    if (lvl.isEmpty())
    {
      throw std::invalid_argument("Initialization string is invalid (empty start level)!");
    }
    
    if (lvl == "L16") startLvl = L16;
    else if (lvl == "Q") startLvl = QUARTER;
    else if (lvl == "S") startLvl = SEMI;
    else
    {
      throw std::invalid_argument("Initialization string is invalid (wrong start level)!");
    }
    
    // try to parse the second field (bool for "secondSurvives)
    QString secondSurv = fields.at(1);
    secondSurv = secondSurv.trimmed();
    if (secondSurv == "0") secondSurvives = false;
    else if (secondSurv == "1") secondSurvives = true;
    else
    {
      throw std::invalid_argument("Initialization string is invalid (tag for 'second survives')!");
    }
    
    // try to parse the group definitions
    int i = 2;
    while ((i+1) < (fields.count()-1))   // "-1" because we always have a trailing ";"
    {
      QString num = fields.at(i).trimmed();
      QString gSize = fields.at(i+1).trimmed();
      
      int numGroups;
      int grpSize;
      try
      {
	numGroups = num.toInt();
	grpSize = gSize.toInt();
      } catch (std::exception ex) {
	throw std::invalid_argument("Initialization string is invalid (non-digits in group def)!");
      }
      
      grpDefs.append(GroupDef(grpSize, numGroups));
    }
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}


/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include <stdexcept>

#include "KO_Config.h"
#include "TournamentDataDefs.h"
#include <QString>
#include <QStringList>

namespace QTournament {

  KO_Config::KO_Config(KO_START _startLevel, bool _secondSurvives, GroupDefList grps)
  {
    startLvl = _startLevel;
    secondSurvives = _secondSurvives;
    
    for (int n=0; n<grps.count(); n++)
    {
      grpDefs.append(grps.at(n));
    }
    
    // If we dive directly into the finals, then we always need the second
    // of each group for the match for 3rd place
    if (startLvl == FINAL) secondSurvives = true;
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

  bool KO_Config::isValid(int opponentCount) const
  {
    // compare the required number with the actual number of groups
    // we must have more than the next lower "KO-Level" requires and not more
    // than the this KO-Level
    int reqGroupCount = getNumReqGroups();
    int prevLevelGroupCount = reqGroupCount / 2;
    int actualGroupCount = grpDefs.getTotalGroupCount();
    if ((actualGroupCount > reqGroupCount) || (actualGroupCount <= prevLevelGroupCount))
    {
      return false;
    }
    
    // if provided by the caller, also check the number of players
    if (opponentCount > 0)
    {
      return (opponentCount == grpDefs.getTotalPlayerCount());
    }
    
    return true;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumMatches() const
  {
    int n = 2;  // Final plus match for third place
    if (startLvl == SEMI) n += 2;   // two semi-finals
    if (startLvl == QUARTER) n += 2 + 4; // semi-finals plus four quarter finals
    if (startLvl == L16) n += 2 + 4 + 8; // ... plus eight matches of the last 16
    
    n += grpDefs.getTotalMatchCount();
    
    return n;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumGroupMatches() const
  {
    return grpDefs.getTotalMatchCount();
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumGroupDefs() const
  {
    return grpDefs.count();
  }

//----------------------------------------------------------------------------

  bool KO_Config::getSecondSurvives() const
  {
    return secondSurvives;
  }

//----------------------------------------------------------------------------

  GroupDef KO_Config::getGroupDef(int i) const
  {
    if ((i < 0) || (i >= grpDefs.count()))
    {
      throw std::invalid_argument("Group index not valid!");
    }
    
    return grpDefs.at(i);
  }

//----------------------------------------------------------------------------

  KO_START KO_Config::getStartLevel() const
  {
    return startLvl;
  }

//----------------------------------------------------------------------------

  QString KO_Config::toString() const
  {
    QString result = "L16";
    
    if (startLvl == SEMI) result = "S";
    else if (startLvl == QUARTER) result = "Q";
    else if (startLvl == FINAL) result = "F";
    
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
    else if (lvl == "F") startLvl = FINAL;
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
      
      i += 2;
    }

    // the last field (after the closing ';') must always be empty
    if (!(fields.last().trimmed().isEmpty()))
    {
      throw std::invalid_argument("Initialization string is invalid (additional data at the end)!");
    }
  }

//----------------------------------------------------------------------------

  void KO_Config::setStartLevel(KO_START newLvl)
  {
    startLvl = newLvl;
  }

//----------------------------------------------------------------------------

  void KO_Config::setSecondSurvives(bool newSurvive)
  {
    secondSurvives = newSurvive;
    if (startLvl == FINAL) secondSurvives = true;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumReqGroups() const
  {
    // calculate the number of required groups,
    // based on the start configuration of the KO rounds
    int reqGroups = 16;
    if (startLvl == FINAL)
    {
      reqGroups = 2;
    }
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
    if ((secondSurvives) && (startLvl != FINAL))
    {
      reqGroups = reqGroups / 2;
    }
    
    return reqGroups;
  }

//----------------------------------------------------------------------------

  GroupDefList KO_Config::getGroupDefList() const
  {
    return grpDefs;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumGroups() const
  {
    int result = 0;
    for (int i=0; i < grpDefs.count(); i++)
    {
      GroupDef g = grpDefs.at(i);
      
      result += g.getNumGroups();
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumRounds() const
  {
    int result = -1;

    // find the max number of rounds
    // among all group definitions
    for (GroupDef g : grpDefs)
    {
      int grpSize = g.getGroupSize();

      // for an even group size, the number of
      // rounds is "size - 1", for odd sizes it's
      // "size" which each player having a bye
      int numRounds = ((grpSize % 2) == 0) ? grpSize-1 : grpSize;

      if (numRounds > result) result = numRounds;
    }

    return result;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}


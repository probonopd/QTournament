/* 
 * File:   KO_Config.cpp
 * Author: volker
 * 
 * Created on August 11, 2014, 7:40 PM
 */

#include "KO_Config.h"
#include "TournamentDataDefs.h"

namespace QTournament {

  KO_Config::KO_Config(KO_START _startLevel, bool _secondSurvives, GroupDef grps[], int _nGroups)
  {
    startLvl = _startLevel;
    secondSurvives = _secondSurvives;
    
    if ((grps == 0) || (_nGroups <= 0))
    {
      grpDefs = 0;
      nGroups = 0;
      return;
    }
    
    nGroups = _nGroups;
    grpDefs = new GroupDef[nGroups];
    for (int n=0; n<nGroups; n++)
    {
      grpDefs[n].copy(grps[n]);
    }
  }

//----------------------------------------------------------------------------

  KO_Config::KO_Config(const KO_Config& orig)
  {
    nGroups = orig.nGroups;
    startLvl = orig.startLvl;
    secondSurvives = orig.secondSurvives;
    
    grpDefs = new GroupDef[nGroups];
    for (int n=0; n<nGroups; n++)
    {
      grpDefs[n].copy(orig.grpDefs[n]);
    }    
  }

//----------------------------------------------------------------------------

  KO_Config::~KO_Config()
  {
    delete[] grpDefs;
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
    return reqGroups == nGroups;
  }

//----------------------------------------------------------------------------

  int KO_Config::getNumMatches()
  {
    int n = 2;  // Final plus match for third place
    if (startLvl == SEMI) n += 2;   // two semi-finals
    if (startLvl == QUARTER) n += 2 + 4; // semi-finals plus four quarter finals
    if (startLvl == L16) n += 2 + 4 + 8; // ... plus eight matches of the last 16
    
    for (int i=0; i<nGroups; i++)
    {
      n += grpDefs[i].getNumMatches();
    }
    
    return n;
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

}


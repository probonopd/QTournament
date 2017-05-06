#include <QString>

#include <Sloppy/libSloppy.h>

#include "CSVImporter.h"
#include "TournamentDB.h"
#include "PlayerMngr.h"
#include "CatMngr.h"

namespace QTournament
{
  SEX strToSex(const string& s)
  {
    if ((s == "m") || (s == "M")) return M;

    for (const string& permitted : {"f", "F", "w", "W"})
    {
      if (s == permitted) return F;
    }

    // use "DONT_CARE" as an error indicator
    return DONT_CARE;
  }

  //----------------------------------------------------------------------------

  vector<vector<string>> splitCSV(const string& rawText, const string& delim)
  {
    vector<vector<string>> result;

    Sloppy::StringList lines;
    Sloppy::stringSplitter(lines, rawText, "\n", true);

    for (const string& line : lines)
    {
      if (line.empty()) continue;

      Sloppy::StringList fields;
      Sloppy::stringSplitter(fields, line, delim, true);

      // remove empty fields at the end
      while (!(fields.empty()))
      {
        if (fields.back().empty()) fields.pop_back();
        else break;
      }
      if (fields.empty()) continue;  // that also captures lines like ",,,,,"

      // in case we have more than one category (more than
      // four fields), we merge all categories into a comma
      // separated list in the fourth field
      if (fields.size() > 4)
      {
        string c;
        for (size_t idx = 4 ; idx < fields.size(); ++idx)
        {
          if (fields[idx].empty()) continue;  // skip empty fields
          c += fields[idx] + ", ";
        }
        if (!(c.empty())) c = c.substr(0, c.size() - 2);

        while (fields.size() > 5) fields.pop_back();  // erase all additional fields

        if (!(c.empty()))
        {
          fields[CSVFieldsIndex::Categories] = c;  // store the comma sep. list...
        } else {
          fields.pop_back();   // ... or remove the fields completely if it is empty
        }
      }

      result.push_back(fields);
    }

    return result;
  }

  //----------------------------------------------------------------------------

  vector<CSVError> analyseCSV(TournamentDB* db, const vector<vector<string>>& data)
  {
    vector<CSVError> result;

    PlayerMngr pm{db};
    CatMngr cm{db};

    int row = 0;
    for (const vector<string>& fields : data)
    {
      // check for the required number of fields
      if ((fields.size() < 4) || (fields[CSVFieldsIndex::Team].empty()))
      {
        CSVError err{row, CSVFieldsIndex::Team, CSVErrCode::NoTeamName, "", true};
        result.push_back(err);
      }
      if ((fields.size() < 3) || (fields[CSVFieldsIndex::Sex].empty()))
      {
        CSVError err{row, CSVFieldsIndex::Sex, CSVErrCode::NoSex, "", true};
        result.push_back(err);
      }
      if ((fields.size() < 2) || (fields[CSVFieldsIndex::FirstName].empty()))
      {
        CSVError err{row, CSVFieldsIndex::FirstName, CSVErrCode::NoFirstName, "", true};
        result.push_back(err);
      }

      // make sure the name is unique
      if (fields.size() >= 2)
      {
        QString f = QString::fromUtf8(fields[CSVFieldsIndex::FirstName].c_str());
        QString l = QString::fromUtf8(fields[CSVFieldsIndex::LastName].c_str());
        if (pm.hasPlayer(f, l))
        {
          CSVError err{row, CSVFieldsIndex::FirstName, CSVErrCode::NameNotUnique, "", true};
          result.push_back(err);
          err = CSVError{row, CSVFieldsIndex::LastName, CSVErrCode::NameNotUnique, "", true};
          result.push_back(err);
        }
      }

      // check for a valid sex indicator
      if (fields.size() >= 3)
      {
        SEX s = strToSex(fields[CSVFieldsIndex::Sex]);
        if (s == DONT_CARE)
        {
          CSVError err{row, CSVFieldsIndex::Sex, CSVErrCode::InvalidSexIndicator, "", true};
          result.push_back(err);
        }
      }

      // check for valid categories
      if (fields.size() > 4)
      {
        string _allCats = fields[CSVFieldsIndex::Categories];
        Sloppy::StringList allCats;
        Sloppy::stringSplitter(allCats, _allCats, ",", true);
        for (const string& cName : allCats)
        {
          // does the category exist?
          if (!(cm.hasCategory(QString::fromUtf8(cName.c_str()))))
          {
            CSVError err{row, CSVFieldsIndex::Categories, CSVErrCode::CategoryNotExisting, cName, false};
            result.push_back(err);

            continue;
          }

          // can players be added to the category?
          Category cat = cm.getCategory(QString::fromUtf8(cName.c_str()));
          SEX s = strToSex(fields[CSVFieldsIndex::Sex]);
          CAT_ADD_STATE as = cat.getAddState(s);
          if (as == CAT_CLOSED)
          {
            CSVError err{row, CSVFieldsIndex::Categories, CSVErrCode::CategoryLocked, cName, false};
            result.push_back(err);
          }
          if ((as != CAT_CLOSED) && (as != CAN_JOIN))
          {
            CSVError err{row, CSVFieldsIndex::Categories, CSVErrCode::CategoryNotSuitable, cName, false};
            result.push_back(err);
          }
        }
      }

      // next line
      ++row;
    }

    return result;
  }

  //----------------------------------------------------------------------------


}

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

  std::vector<vector<string>> splitCSV(const string& rawText, const string& delim, const string& optionalCatName)
  {
    std::vector<vector<string>> result;

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

      Sloppy::StringList catNames;
      if (!(optionalCatName.empty()))
      {
        catNames.push_back(optionalCatName);
      }
      // in case we have more than one category (more than
      // four fields), we merge all categories into a comma
      // separated list in the fourth field
      if (fields.size() > 4)
      {
        for (size_t idx = 4 ; idx < fields.size(); ++idx)
        {
          // skip empty fields
          if (fields[idx].empty()) continue;

          // do not add a category twice
          string cName = fields[idx];
          if (Sloppy::isInVector<string>(catNames, cName)) continue;

          catNames.push_back(cName);
        }

        while (fields.size() > 4) fields.pop_back();  // erase all category fields
      }

      // replace the category column with the content of catNames, if any
      if (!(catNames.empty()))
      {
        while (fields.size() < 4) fields.push_back("");
        fields.push_back(Sloppy::commaSepStringFromStringList(catNames, ", "));  // store the comma sep. list...
      }

      // make sure we have always five columns;
      // otherwise we can't save user provided data
      // from the UI later on
      while (fields.size() < 5) fields.push_back("");

      result.push_back(fields);
    }

    return result;
  }

  //----------------------------------------------------------------------------

  std::vector<CSVImportRecord> convertCSVfromPlainText(TournamentDB* db, const std::vector<vector<string> >& splitData)
  {
    std::vector<CSVImportRecord> result;
    for (const std::vector<string>& s : splitData)
    {
      result.push_back(CSVImportRecord{db, s});
    }

    return result;
  }

  //----------------------------------------------------------------------------

  std::vector<CSVError> analyseCSV(const TournamentDB& db, const std::vector<CSVImportRecord>& data)
  {
    std::vector<CSVError> result;

    PlayerMngr pm{db};
    CatMngr cm{db};

    int row = 0;
    for (const CSVImportRecord& rec : data)
    {
      // check for the required number of fields
      if (!(rec.hasTeamName()))
      {
        CSVError err{row, CSVFieldsIndex::Team, CSVErrCode::NoTeamName, "", true};
        result.push_back(err);
      }
      if (!(rec.hasValidSex()))
      {
        CSVError err{row, CSVFieldsIndex::Sex, CSVErrCode::NoSex, "", true};
        result.push_back(err);
      }
      if (!(rec.hasFirstName()))
      {
        CSVError err{row, CSVFieldsIndex::FirstName, CSVErrCode::NoFirstName, "", true};
        result.push_back(err);
      }
      if (!(rec.hasLastName()))
      {
        CSVError err{row, CSVFieldsIndex::LastName, CSVErrCode::NoLastName, "", true};
        result.push_back(err);
      }

      // check if the name is globally unique
      // (--> not yet in the database)
      if (rec.hasExistingName())
      {
        CSVError err{row, CSVFieldsIndex::FirstName, CSVErrCode::NameNotUnique, "", false};
        result.push_back(err);
        err = CSVError{row, CSVFieldsIndex::LastName, CSVErrCode::NameNotUnique, "", false};
        result.push_back(err);
      }

      // check if the name is locally unique
      // (--> not yet in this list of records)
      if (rec.hasLastName() && rec.hasFirstName())
      {
        for (int earlierRow = 0; earlierRow < row; ++earlierRow)
        {
          const CSVImportRecord& other = data[earlierRow];
          if ((rec.getLastName() == other.getLastName()) && (rec.getFirstName() == other.getFirstName()))
          {
            // generate an error and add 1 to the row number
            // so that it matches the row numbers in the tab widget
            CSVError err{row, CSVFieldsIndex::FirstName, CSVErrCode::NameRedundant, QString::number(earlierRow + 1), true};
            result.push_back(err);
            err = CSVError{row, CSVFieldsIndex::LastName, CSVErrCode::NameRedundant, QString::number(earlierRow + 1), true};
            result.push_back(err);
          }
        }
      }

      // check for valid categories
      if (!(rec.getCatNames().empty()))
      {
        for (const QString& cName : rec.getCatNames())
        {
          // does the category exist?
          if (!(cm.hasCategory(cName)))
          {
            CSVError err{row, CSVFieldsIndex::Categories, CSVErrCode::CategoryNotExisting, cName, false};
            result.push_back(err);

            continue;
          }

          // can players be added to the category?
          Category cat = cm.getCategory(cName);
          if (cat.canAddPlayers())
          {
            CAT_ADD_STATE as = cat.getAddState(rec.getSex());
            if (as != CAN_JOIN)
            {
              CSVError err{row, CSVFieldsIndex::Categories, CSVErrCode::CategoryNotSuitable, cName, false};
              result.push_back(err);
            }
          } else {
            CSVError err{row, CSVFieldsIndex::Categories, CSVErrCode::CategoryLocked, cName, false};
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
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  CSVImportRecord::CSVImportRecord(const TournamentDB& _db, std::vector<string> rawTexts)
    :db{_db}
  {
    // copy the lastname
    if (rawTexts.size() > 0)
    {
      lName = QString::fromUtf8(rawTexts[0].c_str());
    }

    // copy the given name
    if (rawTexts.size() > 1)
    {
      fName = QString::fromUtf8(rawTexts[1].c_str());
    }

    // copy the sex
    if (rawTexts.size() > 2)
    {
      sex = strToSex(rawTexts[2]);
    } else {
      sex = DONT_CARE;
    }

    // copy the team name
    if (rawTexts.size() > 3)
    {
      teamName = QString::fromUtf8(rawTexts[3].c_str());
    }

    // copy the cat names
    if (rawTexts.size() > 4)
    {
      std::vector<string> names;
      Sloppy::stringSplitter(names, rawTexts[4], ",", true);
      for (const string& s : names)
      {
        catNames.push_back(QString::fromUtf8(s.c_str()));
      }
    }

    // if our name matches the name of an existing player,
    // we enforce the correct sex, no matter what the user originally
    // provided as input data
    enforceConsistentSex();

    // if some elements are missing and we're representing
    // an existing player, fill in the correct values from the
    // database
    insertMissingDataForExistingPlayers();
  }

  //----------------------------------------------------------------------------

  void CSVImportRecord::enforceConsistentSex()
  {
    upPlayer p = getExistingPlayer();
    if (p != nullptr)
    {
      sex = p->getSex();
    }
  }

  //----------------------------------------------------------------------------

  void CSVImportRecord::insertMissingDataForExistingPlayers()
  {
    if (!(hasExistingName())) return;

    Player p = *(getExistingPlayer());

    if (teamName.isEmpty())
    {
      teamName = p.getTeam().getName();
    }

    // merge already assigned and potentially new categories
    CatMngr cm{db};
    std::vector<QString> alreadyAssignedCats;
    for (const Category& cat : cm.getAllCategories())
    {
      // skip categories that are already locked
      if (!(cat.canAddPlayers())) continue;

      if (cat.hasPlayer(p))
      {
        alreadyAssignedCats.push_back(cat.getName());
      }
    }
    if (catNames.empty())
    {
      catNames = alreadyAssignedCats;
    } else {
      for (const QString& cn : alreadyAssignedCats)
      {
        if (Sloppy::isInVector<QString>(catNames, cn)) continue;
        catNames.push_back(cn);
      }
    }
  }

  //----------------------------------------------------------------------------

  bool CSVImportRecord::hasExistingName() const
  {
    if (!(hasFirstName()) || !(hasLastName())) return false;

    PlayerMngr pm{db};
    return pm.hasPlayer(fName, lName);
  }

  //----------------------------------------------------------------------------

  QString CSVImportRecord::getCatNames_str() const
  {
    QString result;
    for (const QString& cn : catNames)
    {
      result += cn + ", ";
    }
    if (!(result.isEmpty())) result.chop(2);

    return result;
  }

  //----------------------------------------------------------------------------

  std::optional<Player> CSVImportRecord::getExistingPlayer() const
  {
    PlayerMngr pm{db};
    if (pm.hasPlayer(fName, lName))
    {
      Player p = pm.getPlayer(fName, lName);
      return pm.getPlayer2(p.getId());
    }

    return nullptr;
  }

  //----------------------------------------------------------------------------

  bool CSVImportRecord::updateFirstName(const QString& newName)
  {
    if (newName.isEmpty()) return false;
    fName = newName;
    enforceConsistentSex();

    return true;
  }

  //----------------------------------------------------------------------------

  bool CSVImportRecord::updateLastName(const QString& newName)
  {
    if (newName.isEmpty()) return false;
    lName = newName;
    enforceConsistentSex();

    return true;
  }

  //----------------------------------------------------------------------------

  bool CSVImportRecord::updateTeamName(const QString& newName)
  {
    if (newName.isEmpty()) return false;

    teamName = newName;

    return true;
  }

  //----------------------------------------------------------------------------

  bool CSVImportRecord::updateSex(SEX newSex)
  {
    if (newSex == DONT_CARE) return false;
    if (hasExistingName()) return false;

    sex = newSex;

    return true;
  }

  //----------------------------------------------------------------------------

  bool CSVImportRecord::updateCategories(const std::vector<QString>& catOverwrite)
  {
    if (catOverwrite.empty()) return false;

    catNames = catOverwrite;
    return true;
  }
}

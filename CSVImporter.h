#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

#include <string>
#include <vector>
#include <memory>

#include "TournamentDataDefs.h"
#include "Player.h"

using namespace std;

namespace QTournament
{
  class TournamentDB;

  enum class CSVErrCode
  {
    NoTeamName,
    NoSex,
    NoFirstName,
    NoLastName,
    NameNotUnique,
    NameRedundant,
    CategoryNotExisting,
    CategoryLocked,
    CategoryNotSuitable
  };

  struct CSVError
  {
    int row;
    int column;
    CSVErrCode err;
    QString para;
    bool isFatal;
  };

  struct CSVFieldsIndex
  {
    static constexpr int LastName = 0;
    static constexpr int FirstName = 1;
    static constexpr int Sex = 2;
    static constexpr int Team = 3;
    static constexpr int Categories = 4;
  };

  SEX strToSex(const string& s);

  class CSVImportRecord
  {
  public:
    CSVImportRecord(TournamentDB* _db, vector<string> rawTexts);
    void enforceConsistentSex();
    void insertMissingDataForExistingPlayers();

    // boolean checks
    bool hasLastName() const { return !(lName.isEmpty()); }
    bool hasFirstName() const { return !(fName.isEmpty()); }
    bool hasTeamName() const { return !(teamName.isEmpty()); }
    bool hasValidSex() const { return (sex != DONT_CARE); }
    bool hasExistingName() const;

    // getters
    vector<QString> getCatNames() const { return catNames; }
    QString getCatNames_str() const;
    SEX getSex() const { return sex; }
    QString getLastName() const { return lName; }
    QString getFirstName() const { return fName; }
    QString getTeamName() const { return teamName; }

    unique_ptr<QTournament::Player> getExistingPlayer() const;

    // setters
    bool updateFirstName(const QString& newName);
    bool updateLastName(const QString& newName);
    bool updateTeamName(const QString& newName);
    bool updateSex(SEX newSex);
    bool updateCategories(const vector<QString>& catOverwrite);

  private:
    TournamentDB* db;
    QString fName;
    QString lName;
    SEX sex;
    QString teamName;
    vector<QString> catNames;
  };

  vector<vector<string>> splitCSV(const string& rawText, const string& delim = ",", const string& optionalCatName="");
  vector<CSVImportRecord> convertCSVfromPlainText(TournamentDB* db, const vector<vector<string>>& splitData);
  vector<CSVError> analyseCSV(TournamentDB* db, const vector<CSVImportRecord>& data);

}

#endif // CSVIMPORTER_H

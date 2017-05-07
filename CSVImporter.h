#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

#include <string>
#include <vector>

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
    InvalidSexIndicator,
    CategoryNotExisting,
    CategoryLocked,
    CategoryNotSuitable
  };

  struct CSVError
  {
    int row;
    int column;
    CSVErrCode err;
    string para;
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

  vector<vector<string>> splitCSV(const string& rawText, const string& delim = ",", const string& optionalCatName="");

  vector<CSVError> analyseCSV(TournamentDB* db, const vector<vector<string>>& data);

}

#endif // CSVIMPORTER_H

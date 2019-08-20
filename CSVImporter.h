#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

#include <string>
#include <vector>
#include <memory>

#include "TournamentDataDefs.h"
#include "Player.h"

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

  Sex strToSex(const std::string& s);

  class CSVImportRecord
  {
  public:
    CSVImportRecord(const TournamentDB& _db, std::vector<Sloppy::estring> rawTexts);
    void enforceConsistentSex();
    void insertMissingDataForExistingPlayers();

    // boolean checks
    bool hasLastName() const { return !(lName.isEmpty()); }
    bool hasFirstName() const { return !(fName.isEmpty()); }
    bool hasTeamName() const { return !(teamName.isEmpty()); }
    bool hasValidSex() const { return (sex != Sex::DontCare); }
    bool hasExistingName() const;

    // getters
    std::vector<QString> getCatNames() const { return catNames; }
    QString getCatNames_str() const;
    Sex getSex() const { return sex; }
    QString getLastName() const { return lName; }
    QString getFirstName() const { return fName; }
    QString getTeamName() const { return teamName; }

    std::optional<Player> getExistingPlayer() const;

    // setters
    bool updateFirstName(const QString& newName);
    bool updateLastName(const QString& newName);
    bool updateTeamName(const QString& newName);
    bool updateSex(Sex newSex);
    bool updateCategories(const std::vector<QString>& catOverwrite);

  private:
    std::reference_wrapper<const QTournament::TournamentDB> db;
    QString fName;
    QString lName;
    Sex sex;
    QString teamName;
    std::vector<QString> catNames;
  };

  std::vector<std::vector<Sloppy::estring> > splitCSV(const Sloppy::estring& rawText, const std::string& delim = ",", const std::string& optionalCatName="");
  std::vector<CSVImportRecord> convertCSVfromPlainText(const TournamentDB& db, const std::vector<std::vector<Sloppy::estring> >& splitData);
  std::vector<CSVError> analyseCSV(const TournamentDB& db, const std::vector<CSVImportRecord>& data);

}

#endif // CSVIMPORTER_H

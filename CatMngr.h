/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#ifndef CATMNGR_H
#define	CATMNGR_H

#include <memory>

#include <QList>
#include <QHash>

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include <SqliteOverlay/DbTab.h>
#include "TournamentDatabaseObjectManager.h"
#include "Category.h"
#include "PlayerPair.h"

namespace QTournament
{

  class CatMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    // constructor
    CatMngr (const TournamentDB& _db);

    // creation of categories
    Error createNewCategory (const QString& catName);
    Error cloneCategory(const Category& src, const QString& catNamePostfix);

    // boolean queries
    bool hasCategory (const QString& catName) const;

    // getters
    Category getCategory(const QString& name);
    std::optional<Category> getCategory(int id);
    Category getCategoryById(int id);
    Category getCategoryBySeqNum(int seqNum);
    CategoryList getAllCategories();
    QHash<Category, CatAddState> getAllCategoryAddStates(Sex s);
    QHash<Category, CatAddState> getAllCategoryAddStates(const Player& p);
    static std::function<bool (Category&, Category&)> getCategorySortFunction_byName();
    std::vector<PlayerPair> getSeeding(const Category& cat) const;
    Error canDeleteCategory(const Category& cat) const;

    // setters
    Error setMatchType(Category& cat, MatchType newMatchType);
    Error setMatchSystem(Category& cat, MatchSystem newMatchSystem);
    Error setSex(Category& cat, Sex newSex);
    bool setCatParameter(Category& cat, CatParameter p, const QVariant& v);

    // modifications
    Error renameCategory(Category& cat, const QString& newName);
    Error addPlayerToCategory(const Player& p, const Category& cat);
    Error removePlayerFromCategory(const Player& p, const Category& cat) const;
    Error deleteCategory(const Category& cat) const;
    Error deleteRunningCategory(const Category& cat) const;

    // pairing
    Error pairPlayers(const Category c, const Player& p1, const Player& p2);
    Error splitPlayers(const Category c, const Player& p1, const Player& p2) const;
    Error splitPlayers(const Category c, int pairId) const;

    // freezing, starting, updating while running
    Error freezeConfig(const Category& c);
    Error unfreezeConfig(const Category& c);
    Error startCategory(const Category& c, const std::vector<PlayerPairList>& grpCfg, const PlayerPairList& seed);
    void updateCatStatusFromMatchStatus(const Category& cat);
    bool switchCatToWaitForSeeding(const Category& cat);
    Error continueWithIntermediateSeeding(const Category& c, const PlayerPairList& seeding);

    std::string getSyncString(const std::vector<int>& rows) const override;

  private:
    bool setCatParam_AllowDraw( Category& c, const QVariant& v);
    bool setCatParam_Score( Category& c, int newScore, bool isDraw);
  };
}

#endif	/* TEAMMNGR_H */


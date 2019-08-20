/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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
    ERR createNewCategory (const QString& catName);
    ERR cloneCategory(const Category& src, const QString& catNamePostfix);

    // boolean queries
    bool hasCategory (const QString& catName) const;

    // getters
    Category getCategory(const QString& name);
    std::optional<Category> getCategory(int id);
    Category getCategoryById(int id);
    Category getCategoryBySeqNum(int seqNum);
    CategoryList getAllCategories();
    QHash<Category, CAT_ADD_STATE> getAllCategoryAddStates(SEX s);
    QHash<Category, CAT_ADD_STATE> getAllCategoryAddStates(const Player& p);
    static std::function<bool (Category&, Category&)> getCategorySortFunction_byName();
    std::vector<PlayerPair> getSeeding(const Category& cat) const;
    ERR canDeleteCategory(const Category& cat) const;

    // setters
    ERR setMatchType(Category& cat, MATCH_TYPE newMatchType);
    ERR setMatchSystem(Category& cat, MATCH_SYSTEM newMatchSystem);
    ERR setSex(Category& cat, SEX newSex);
    bool setCatParameter(Category& cat, CatParameter p, const QVariant& v);

    // modifications
    ERR renameCategory(Category& cat, const QString& newName);
    ERR addPlayerToCategory(const Player& p, const Category& cat);
    ERR removePlayerFromCategory(const Player& p, const Category& cat) const;
    ERR deleteCategory(const Category& cat) const;
    ERR deleteRunningCategory(const Category& cat) const;

    // pairing
    ERR pairPlayers(const Category c, const Player& p1, const Player& p2);
    ERR splitPlayers(const Category c, const Player& p1, const Player& p2) const;
    ERR splitPlayers(const Category c, int pairId) const;

    // freezing, starting, updating while running
    ERR freezeConfig(const Category& c);
    ERR unfreezeConfig(const Category& c);
    ERR startCategory(const Category& c, const std::vector<PlayerPairList>& grpCfg, const PlayerPairList& seed);
    void updateCatStatusFromMatchStatus(const Category& cat);
    bool switchCatToWaitForSeeding(const Category& cat);
    ERR continueWithIntermediateSeeding(const Category& c, const PlayerPairList& seeding);

    std::string getSyncString(const std::vector<int>& rows) const override;

  private:
    bool setCatParam_CatParameter::AllowDraw( Category& c, const QVariant& v);
    bool setCatParam_Score( Category& c, int newScore, bool isDraw);
  };
}

#endif	/* TEAMMNGR_H */


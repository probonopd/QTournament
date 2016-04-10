/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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
#include "DbTab.h"
#include "TournamentDatabaseObjectManager.h"
#include "Category.h"
#include "PlayerPair.h"
#include "ThreadSafeQueue.h"

namespace QTournament
{

  class CatMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    // constructor
    CatMngr (TournamentDB* _db);

    // creation of categories
    ERR createNewCategory (const QString& catName);
    ERR cloneCategory(const Category& src, const QString& catNamePostfix);

    // boolean queries
    bool hasCategory (const QString& catName) const;

    // getters
    Category getCategory(const QString& name);
    unique_ptr<Category> getCategory(int id);
    Category getCategoryById(int id);
    Category getCategoryBySeqNum(int seqNum);
    vector<Category> getAllCategories();
    QHash<Category, CAT_ADD_STATE> getAllCategoryAddStates(SEX s);
    QHash<Category, CAT_ADD_STATE> getAllCategoryAddStates(const Player& p);
    static std::function<bool (Category&, Category&)> getCategorySortFunction_byName();
    vector<PlayerPair> getSeeding(const Category& c) const;
    ERR canDeleteCategory(const Category& cat) const;

    // setters
    ERR setMatchType(Category& c, MATCH_TYPE t);
    ERR setMatchSystem(Category& c, MATCH_SYSTEM s);
    ERR setSex(Category& c, SEX s);
    bool setCatParameter( Category& c, CAT_PARAMETER p, const QVariant& v);

    // modifications
    ERR renameCategory(Category& c, const QString& newName);
    ERR addPlayerToCategory(const Player& p, const Category& c);
    ERR removePlayerFromCategory(const Player& p, const Category& c) const;
    ERR deleteCategory(const Category& cat) const;
    ERR deleteRunningCategory(const Category& cat) const;

    // pairing
    ERR pairPlayers(const Category c, const Player& p1, const Player& p2);
    ERR splitPlayers(const Category c, const Player& p1, const Player& p2) const;
    ERR splitPlayers(const Category c, int pairId) const;

    // freezing, starting, updating while running
    ERR freezeConfig(const Category& c);
    ERR unfreezeConfig(const Category& c);
    ERR startCategory(const Category& c, vector<PlayerPairList> grpCfg, PlayerPairList seed, ProgressQueue* progressNotificationQueue=nullptr);
    void updateCatStatusFromMatchStatus(const Category& c);
    bool switchCatToWaitForSeeding(const Category& cat);
    ERR continueWithIntermediateSeeding(const Category& c, const PlayerPairList& seeding, ProgressQueue* progressNotificationQueue=nullptr);

  private:
    bool setCatParam_AllowDraw( Category& c, const QVariant& v);
    bool setCatParam_Score( Category& c, int newScore, bool isDraw);
  };
}

#endif	/* TEAMMNGR_H */


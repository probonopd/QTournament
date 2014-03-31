/* 
 * File:   CatMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef CATMNGR_H
#define	CATMNGR_H

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"
#include "Category.h"
#include "PlayerPair.h"

#include <QList>
#include <QHash>

using namespace dbOverlay;

namespace QTournament
{

  class CatMngr : public QObject, GenericObjectManager
  {
    Q_OBJECT
    
  public:
    CatMngr (TournamentDB* _db);
    ERR createNewCategory (const QString& catName);
    bool hasCategory (const QString& catName);
    Category getCategory(const QString& name);
    Category getCategoryById(int id);
    Category getCategoryBySeqNum(int seqNum);
    QList<Category> getAllCategories();
    ERR renameCategory(const Category& c, const QString& newName);
    ERR setMatchType(Category& c, MATCH_TYPE t);
    ERR setMatchSystem(Category& c, MATCH_SYSTEM s);
    ERR setSex(Category& c, SEX s);
    ERR addPlayerToCategory(const Player& p, const Category& c);
    ERR removePlayerFromCategory(const Player& p, const Category& c);
    QHash<Category, CAT_ADD_STATE> getAllCategoryAddStates(SEX s);
    QHash<Category, CAT_ADD_STATE> getAllCategoryAddStates(const Player& p);
    bool setCatParameter( Category& c, CAT_PARAMETER p, const QVariant& v);
    ERR pairPlayers(const Category c, const Player& p1, const Player& p2);
    ERR splitPlayers(const Category c, const Player& p1, const Player& p2);
    ERR splitPlayers(const Category c, int pairId);

  signals:
    void playersPaired(const Category c, const Player& p1, const Player& p2);
    void playersSplit(const Category c, const Player& p1, const Player& p2);
    void playerAddedToCategory(const Player& p, const Category& c);
    void playerRemovedFromCategory(const Player& p, const Category& c);

  private:
    DbTab catTab;
    
    bool setCatParam_AllowDraw( Category& c, const QVariant& v);
    bool setCatParam_Score( Category& c, int newScore, bool isDraw);
  };
}

#endif	/* TEAMMNGR_H */


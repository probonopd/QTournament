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

#include <QList>

using namespace dbOverlay;

namespace QTournament
{

  class CatMngr : public GenericObjectManager
  {
  public:
    CatMngr (TournamentDB* _db);
    ERR createNewCategory (const QString& catName);
    bool hasCategory (const QString& catName);
    Category getCategory(const QString& name);
    QList<Category> getAllCategories();
    ERR renameCategory(const Category& c, const QString& newName);
    ERR setMatchType(Category& c, MATCH_TYPE t);
    ERR setMatchSystem(Category& c, MATCH_SYSTEM s);
    ERR setSex(Category& c, SEX s);

  private:
    DbTab catTab;
  };
}

#endif	/* TEAMMNGR_H */


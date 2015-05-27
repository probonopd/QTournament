/* 
 * File:   GenericObjectManager.h
 * Author: volker
 *
 * Created on March 2, 2014, 8:27 PM
 */

#ifndef GENERICOBJECTMANAGER_H
#define	GENERICOBJECTMANAGER_H

#include "KeyValueTab.h"
#include "TournamentDB.h"

using namespace dbOverlay;

namespace QTournament
{
  class GenericObjectManager
  {
  public:
    GenericObjectManager (TournamentDB* _db);
    TournamentDB* getDatabaseHandle();
    
  protected:
    void fixSeqNumberAfterInsert(const QString& tabName) const;
    void fixSeqNumberAfterDelete(const QString& tabName, int deletedSeqNum) const;

  protected:
    TournamentDB* db;
    KeyValueTab cfg;

    template<class T>
    QList<T> getObjectsByColumnValue(const DbTab& objectTab, const QVariantList& qvl) const
    {
      DbTab::CachingRowIterator it = objectTab.getRowsByColumnValue(qvl);
      return iterator2Objects<T>(it);
    }

    template<class T>
    QList<T> getObjectsByColumnValue(const DbTab& objectTab, const QString& colName, const QVariant& val) const
    {
      DbTab::CachingRowIterator it = objectTab.getRowsByColumnValue(colName, val);
      return iterator2Objects<T>(it);
    }

    template<class T>
    QList<T> getObjectsByWhereClause(const DbTab& objectTab, const QString& where, const QVariantList& args=QVariantList()) const
    {
      DbTab::CachingRowIterator it = objectTab.getRowsByWhereClause(where, args);
      return iterator2Objects<T>(it);
    }

    template<class T>
    QList<T> getAllObjects(const DbTab& objectTab) const
    {
      DbTab::CachingRowIterator it = objectTab.getAllRows();
      return iterator2Objects<T>(it);
    }

    template<class T>
    QList<T> iterator2Objects(DbTab::CachingRowIterator it) const
    {
      QList<T> result;
      while (!(it.isEnd()))
      {
        result.append(T(db, *it));
        ++it;
      }
      return result;
    }

    template<class T>
    unique_ptr<T> getSingleObjectByColumnValue(const DbTab& objectTab, const QVariantList& qvl) const
    {
      try
      {
        TabRow r = objectTab.getSingleRowByColumnValue(qvl);
        return unique_ptr<T>(new T(db, r));
      } catch (std::exception e) {
      }
      return nullptr;
    }

    template<class T>
    unique_ptr<T> getSingleObjectByWhereClause(const DbTab& objectTab, const QString& where, const QVariantList& args=QVariantList()) const
    {
      try
      {
        TabRow r = objectTab.getSingleRowByWhereClause(where, args);
        return unique_ptr<T>(new T(db, r));
      } catch (std::exception e) {
      }
      return nullptr;
    }
  };

}

#endif	/* GENERICOBJECTMANAGER_H */


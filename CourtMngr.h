/* 
 * File:   TeamMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef COURTMNGR_H
#define	COURTMNGR_H

#include <memory>

#include <QList>
#include <QObject>

#include "TournamentDB.h"
#include "Court.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"


using namespace dbOverlay;

namespace QTournament
{

  class CourtMngr : public QObject, GenericObjectManager
  {
    Q_OBJECT
    
  public:
    CourtMngr (TournamentDB* _db);
    unique_ptr<Court> createNewCourt (const int courtNum, const QString& _name, ERR *err);
    bool hasCourt (const int courtNum);
    unique_ptr<Court> getCourt(const int courtNum);
    QList<Court> getAllCourts();
    ERR renameCourt (Court& c, const QString& _newName);
    unique_ptr<Court> getCourtBySeqNum(int seqNum);
    bool hasCourtById(int id);
    unique_ptr<Court> getCourtById(int id);

    unique_ptr<Court> getNextUnusedCourt(bool includeManual=false);

  private:
    DbTab courtTab;

  signals:
    void beginCreateCourt ();
    void endCreateCourt (int newCourtSeqNum);
    void courtRenamed (const Court& p);
  };
}

#endif	/* COURTMNGR_H */


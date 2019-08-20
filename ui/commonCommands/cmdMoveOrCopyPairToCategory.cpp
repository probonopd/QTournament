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

#include <QObject>
#include <QMessageBox>

#include "cmdMoveOrCopyPairToCategory.h"
#include "cmdMoveOrCopyPlayerToCategory.h"
#include "ui/DlgSelectPlayer.h"
#include "CatMngr.h"

cmdMoveOrCopyPairToCategory::cmdMoveOrCopyPairToCategory(QWidget* p, const PlayerPair& _pp, const Category& _srcCat, const Category& _dstCat, bool _isMove)
  :AbstractCommand(_srcCat.getDatabaseHandle(), p), pp(_pp), srcCat(_srcCat), dstCat(_dstCat), isMove(_isMove)
{

}

//----------------------------------------------------------------------------

Error cmdMoveOrCopyPairToCategory::exec()
{
  CatMngr cm{db};

  // check that this is a "true" pair with two players
  if (!(pp.hasPlayer2()))
  {
    // handle this a copy / move action for a single player
    cmdMoveOrCopyPlayerToCategory cmd{parentWidget, pp.getPlayer1(), srcCat, dstCat, isMove};
    return cmd.exec();
  }

  Player p1 = pp.getPlayer1();
  Player p2 = pp.getPlayer2();

  // check if the player pair is in the source category
  auto ppCat = pp.getCategory(db);
  if (ppCat == nullptr)
  {
    QString msg = tr("The provided player pair is invalid.");
    QMessageBox::warning(parentWidget, tr("Copy or move player pair"), msg);
    return Error::InvalidPlayerPair;
  }
  if (srcCat != (*ppCat))
  {
    QString msg = tr("The pair is not assigned to the source category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move or copy player pair"), msg);
    return Error::InvalidPlayerPair;
  }

  // check if the target category is singles category
  if (dstCat.getMatchType() == MatchType::Singles)
  {
    QString msg = tr("Cannot move or copy pair to a singles category");
    QMessageBox::warning(parentWidget, tr("Move or copy player pair"), msg);
    return NoCategoryForPairing;
  }

  // if this is a move operation: make sure we can actually delete
  // the pair from the source category
  if (isMove)
  {
    bool splitFails = (srcCat.canSplitPlayers(p1, p2) != Error::OK);
    bool removeP1Fails = (srcCat.canRemovePlayer(p1) == false);
    bool removeP2Fails = (srcCat.canRemovePlayer(p2) == false);

    if (splitFails || removeP1Fails || removeP2Fails)
    {
      QString msg = tr("The pair cannot be removed from the source category of this operation.");
      QMessageBox::warning(parentWidget, tr("Move player pair"), msg);
      return Error::InvalidPlayerPair;
    }
  }

  // make sure we can add the two players to the target category
  if ((dstCat.getAddState(p1.getSex()) != CatAddState::CanJoin) || (dstCat.getAddState(p2.getSex()) != CatAddState::CanJoin))
  {
    QString msg = tr("Cannot add one or both players to the target category.");
    QMessageBox::warning(parentWidget, tr("Move or copy player pair"), msg);
    return Error::InvalidPlayerPair;
  }

  // transfer the two players in two separate operations
  cmdMoveOrCopyPlayerToCategory cmd1{parentWidget, p1, srcCat, dstCat, isMove};
  Error err = cmd1.exec();
  if (err != Error::OK) return err;   // after all previous checks, this shouldn't happen
  cmdMoveOrCopyPlayerToCategory cmd2{parentWidget, p2, srcCat, dstCat, isMove};
  err = cmd2.exec();
  if (err != Error::OK) return err;   // after all previous checks, this shouldn't happen


  //
  // All that's left to do is to pair the two players in the target category as well
  //
  bool p1IsPaired = dstCat.isPaired(p1);
  bool p2IsPaired = dstCat.isPaired(p2);
  if (p1IsPaired || p2IsPaired)
  {
    QString msg = tr("One or both players already have a partner\n");
    msg += tr("in the target category.\n\n");
    msg += tr("Do you want to replace the existing partners in the\n");
    msg += tr("target category and keep the player pair as it is?");
    int result = QMessageBox::question(parentWidget, tr("Move or copy player pair"), msg);

    if (result == QMessageBox::No)
    {
      // nothing more to do for us
      return Error::OK;
    }
  }

  if (p1IsPaired)
  {
    Player p1Partner = dstCat.getPartner(p1);
    if (p1Partner == p2)   // lucky coincidence: both are already existing and partnered
    {
      return Error::OK;
    }

    if (dstCat.canSplitPlayers(p1, p1Partner) != Error::OK)
    {
      QString msg = tr("Cannot replace the partner of %1\n");
      msg += tr("in the target category. The players have been\n");
      msg += tr("moved / copied but they are not paired.");
      msg = msg.arg(p1.getDisplayName_FirstNameFirst());
      QMessageBox::warning(parentWidget, tr("Move or copy player pair"), msg);
      return Error::OK;   // no error indication in this case
    }

    err = cm.splitPlayers(dstCat, p1, p1Partner);
    if (err != Error::OK) return err;    // shouldn't happen after the previous check
  }

  if (p2IsPaired)
  {
    Player p2Partner = dstCat.getPartner(p2);

    if (dstCat.canSplitPlayers(p2, p2Partner) != Error::OK)
    {
      QString msg = tr("Cannot replace the partner of %1\n");
      msg += tr("in the target category. The players have been\n");
      msg += tr("moved / copied but they are not paired.");
      msg = msg.arg(p2.getDisplayName_FirstNameFirst());
      QMessageBox::warning(parentWidget, tr("Move or copy player pair"), msg);
      return Error::OK;   // no error indication in this case
    }
    err = cm.splitPlayers(dstCat, p2, p2Partner);
    if (err != Error::OK) return err;    // shouldn't happen after the previous check
  }

  // At this point, the two players are free to be paired

  if (cm.pairPlayers(dstCat, p1, p2) != Error::OK)
  {
    QString msg = tr("Cannot pair both players in the target category.");
    msg += tr("The players have been moved / copied but they are not paired.");
    QMessageBox::warning(parentWidget, tr("Move or copy player pair"), msg);

    return err;
  }

  return Error::OK;
}


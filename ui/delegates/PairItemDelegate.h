/* 
 * File:   PlayerItemDelegate.h
 * Author: volker
 *
 * Created on September 7, 2014, 1:26 PM
 */

#ifndef PAIRITEMDELEGATE_H
#define	PAIRITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>

#include "TournamentDB.h"

using namespace QTournament;

class PairItemDelegate : public QStyledItemDelegate
{
public:
  PairItemDelegate(TournamentDB* _db, QObject* parent = nullptr, bool _showListIndex = false);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  
private:
  TournamentDB* db;
  QFontMetrics fntMetrics;
  bool showListIndex;
} ;

#endif	/* PAIRITEMDELEGATE_H */


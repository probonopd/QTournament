/* 
 * File:   PlayerItemDelegate.h
 * Author: volker
 *
 * Created on September 7, 2014, 1:26 PM
 */

#ifndef PLAYERITEMDELEGATE_H
#define	PLAYERITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QAbstractProxyModel>

#include "TournamentDB.h"

using namespace QTournament;

class PlayerItemDelegate : public QStyledItemDelegate
{
public:
  PlayerItemDelegate(TournamentDB* _db, QObject* parent = 0);
  void setProxy(QAbstractProxyModel* _proxy);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  
private:
  TournamentDB* db;
  QAbstractProxyModel* proxy;
  QFontMetrics fntMetrics;
} ;

#endif	/* PLAYERITEMDELEGATE_H */


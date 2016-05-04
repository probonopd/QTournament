/* 
 * File:   PlayerItemDelegate.h
 * Author: volker
 *
 * Created on September 7, 2014, 1:26 PM
 */

#ifndef MATCHITEMDELEGATE_H
#define	MATCHITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QAbstractProxyModel>

#include "TournamentDB.h"

using namespace QTournament;

class MatchItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  static constexpr int ITEM_ROW_HEIGHT = 30;
  static constexpr int ITEM_ROW_HEIGHT_SELECTED = 90;
  static constexpr int ITEM_STAT_INDICATOR_SIZE = 15;
  static constexpr int ITEM_MARGIN = 5;

  MatchItemDelegate(TournamentDB* _db, QObject* parent = 0);
  void setProxy(QAbstractProxyModel* _proxy);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  void setSelectedRow(int _selRow);
  
private:
  TournamentDB* db;
  QAbstractProxyModel* proxy;
  QFontMetrics fntMetrics;
  int selectedRow;
} ;

#endif	/* MATCHITEMDELEGATE_H */


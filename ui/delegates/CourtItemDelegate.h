/* 
 * File:   PlayerItemDelegate.h
 * Author: volker
 *
 * Created on September 7, 2014, 1:26 PM
 */

#ifndef COURTITEMDELEGATE_H
#define	COURTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QAbstractProxyModel>

#include "Match.h"
#include "TournamentDB.h"

using namespace QTournament;

class CourtItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  static constexpr int ITEM_ROW_HEIGHT = 30;
  static constexpr int ITEM_ROW_HEIGHT_SELECTED = 120;
  static constexpr int ITEM_STAT_INDICATOR_SIZE = 15;
  static constexpr int ITEM_MARGIN = 5;

  static constexpr int ITEM_TEXT_ROW_HEIGHT = 15;
  static constexpr int ITEM_TEXT_ROW_SKIP = 5;

  CourtItemDelegate(TournamentDB* _db, QObject* parent = 0);
  void setProxy(QAbstractProxyModel* _proxy);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  void setSelectedRow(int _selRow);
  
private:
  TournamentDB* db;
  QAbstractProxyModel* proxy;
  QFontMetrics fntMetrics;
  int selectedRow;

  void paintMatchInfoCell_Unselected(QPainter* painter, const QStyleOptionViewItem& option, const Match& ma) const;
  void paintMatchInfoCell_Selected(QPainter* painter, const QStyleOptionViewItem& option, const Match& ma) const;
} ;

#endif	/* COURTITEMDELEGATE_H */


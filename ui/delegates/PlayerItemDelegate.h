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

class PlayerItemDelegate : public QStyledItemDelegate
{
public:
  PlayerItemDelegate(QObject* parent = 0);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  
private:
  QFontMetrics fntMetrics;
} ;

#endif	/* PLAYERITEMDELEGATE_H */


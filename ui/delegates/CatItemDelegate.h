/* 
 * File:   PlayerItemDelegate.h
 * Author: volker
 *
 * Created on September 7, 2014, 1:26 PM
 */

#ifndef CATITEMDELEGATE_H
#define	CATITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>

class CatItemDelegate : public QStyledItemDelegate
{
public:
  CatItemDelegate(QObject* parent = 0);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  
private:
  QFontMetrics fntMetrics;
} ;

#endif	/* CATITEMDELEGATE_H */


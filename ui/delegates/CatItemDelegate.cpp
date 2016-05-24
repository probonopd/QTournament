/* 
 * File:   PlayerItemDelegate.cpp
 * Author: volker
 * 
 * Created on September 7, 2014, 1:26 PM
 */

#include "CatItemDelegate.h"

#include <QPainter>

#include "Category.h"
#include "DelegateItemLED.h"
#include "CatMngr.h"

#define CAT_ITEM_ROW_HEIGHT 30
#define CAT_ITEM_STAT_INDICATOR_SIZE 15
#define CAT_ITEM_MARGIN 5

using namespace QTournament;

CatItemDelegate::CatItemDelegate(TournamentDB* _db, QObject* parent)
: QStyledItemDelegate(parent), db(_db), proxy(nullptr), fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

void CatItemDelegate::setProxy(QAbstractProxyModel* _proxy)
{
  proxy = _proxy;
}

//----------------------------------------------------------------------------

void CatItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  if (proxy)
  {
    row = (proxy->mapToSource(index)).row();
  }

  CatMngr cm{db};
  Category c = cm.getCategoryBySeqNum(row);

  painter->save();
  
  // Paint the background in the selection color, if necessary
  QColor bgColor = option.palette.color(QPalette::Background);
  if(option.state & QStyle::State_Selected)
  {
    bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);

    // draw text in highlighted cells in white bold text
    painter->setPen(QPen(QColor(Qt::white)));
    QFont fnt;
    fnt.setBold(true);
    painter->setFont(fnt);
  }
  
  QRect r = option.rect;
  
  // paint logic for the first column, the name
  if (index.column() == 0)
  {
    // draw a status indicator ("LED light")
    DelegateItemLED{}(painter, r, CAT_ITEM_MARGIN, CAT_ITEM_STAT_INDICATOR_SIZE, c.getState(), Qt::lightGray);
    
    // draw the name
    r.adjust(2 * CAT_ITEM_MARGIN + CAT_ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
    QString txt = c.getName();
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
 
  painter->restore();
}

//----------------------------------------------------------------------------

QSize CatItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * CAT_ITEM_MARGIN;
  
  if (index.column() == 0)
  {
    width += CAT_ITEM_MARGIN + CAT_ITEM_STAT_INDICATOR_SIZE;
  }
  
  return QSize(width, CAT_ITEM_ROW_HEIGHT);
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    


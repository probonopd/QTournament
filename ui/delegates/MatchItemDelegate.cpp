/* 
 * File:   MatchItemDelegate.cpp
 * Author: volker
 * 
 * Created on September 7, 2014, 1:26 PM
 */

#include "MatchItemDelegate.h"

#include <QPainter>

#include "Tournament.h"
#include "Match.h"
#include "DelegateItemLED.h"

using namespace QTournament;

MatchItemDelegate::MatchItemDelegate(QObject* parent)
: QStyledItemDelegate(parent), proxy(nullptr), fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

void MatchItemDelegate::setProxy(QAbstractProxyModel *_proxy)
{
  proxy = _proxy;
}

//----------------------------------------------------------------------------

void MatchItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  if (proxy)
  {
    row = (proxy->mapToSource(index)).row();
  }
  auto ma = Tournament::getMatchMngr()->getMatchBySeqNum(row);
  
  // Fill the cell with the selection color, if necessary
  if(option.state & QStyle::State_Selected)
  {
    QColor bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);
  }
  
  QRect r = option.rect;
  
  // paint logic for the second column, the match description
  if (index.column() == 1)
  {
    // draw a status indicator ("LED light")
    DelegateItemLED{}(painter, r, ITEM_MARGIN, ITEM_STAT_INDICATOR_SIZE, ma->getState(), Qt::blue);

    // draw the name
    r.adjust(2 * ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
    QString txt = ma->getDisplayName(tr("Winner"), tr("Loser"));
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
 
  
}

//----------------------------------------------------------------------------

QSize MatchItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * ITEM_MARGIN;
  
  if (index.column() == 1)
  {
    width += ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE;
  }
  
  // this doesn't work, because option.state is not yet updated
  // to QtStyle::State_Selected when sizeHint is called for a freshly
  // selected item
  //int height = (option.state & QStyle::State_Selected) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  int row = index.row();
  //int height = (row == selectedRow) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;
  int height = ITEM_ROW_HEIGHT;

  return QSize(width, height);
}

//----------------------------------------------------------------------------

void MatchItemDelegate::setSelectedRow(int _selRow)
{
  selectedRow = _selRow;
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
    


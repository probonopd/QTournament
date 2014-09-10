/* 
 * File:   PlayerItemDelegate.cpp
 * Author: volker
 * 
 * Created on September 7, 2014, 1:26 PM
 */

#include "PairItemDelegate.h"

#include <QtGlobal>
#include <QPainter>
#include <QFont>

#include "Tournament.h"
#include "PlayerPair.h"

#define PAIR_ITEM_ROW_HEIGHT 40
#define PAIR_ITEM_PLAYERNAME_COL 0, 0, 0
#define PAIR_ITEM_TEAMNAME_COL__REGULAR 0, 0, 255
#define PAIR_ITEM_TEAMNAME_COL__HIGHLIGHT 255, 255, 255
#define PAIR_ITEM_STAT_INDICATOR_SIZE 15
#define PAIR_ITEM_MARGIN 5
#define PAIR_ITEM_TEAMNAME_SHRINK_FAC 0.8

using namespace QTournament;

PairItemDelegate::PairItemDelegate(QObject* parent)
: QStyledItemDelegate(parent), fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

void PairItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  PlayerPair pp = Tournament::getPlayerMngr()->getPlayerPair(index.data(Qt::UserRole).toInt());
  QString playerName = pp.getDisplayName();
  QString teamName = pp.getDisplayName_Team();
  
  // Paint the background in the selection color if necessary
  QColor teamFontColor(PAIR_ITEM_TEAMNAME_COL__REGULAR);
  if(option.state & QStyle::State_Selected)
  {
    painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    teamFontColor = QColor(PAIR_ITEM_TEAMNAME_COL__HIGHLIGHT);
  }
  
  QRect r = option.rect;
  QRect rPlayerName = r.adjusted(PAIR_ITEM_MARGIN, PAIR_ITEM_MARGIN, -PAIR_ITEM_MARGIN, -r.height() / 2);
  painter->drawText(rPlayerName, Qt::AlignVCenter|Qt::AlignLeft, playerName);
  
  QFont teamFont = QFont();
  teamFont.setItalic(true);
  teamFont.setPointSizeF(teamFont.pointSizeF() * PAIR_ITEM_TEAMNAME_SHRINK_FAC);
  painter->save();
  painter->setPen(QPen(teamFontColor));
  painter->setFont(teamFont);
  QRect rTeamName = r.adjusted(PAIR_ITEM_MARGIN, r.height() / 2, -PAIR_ITEM_MARGIN, -PAIR_ITEM_MARGIN);
  painter->drawText(rTeamName, Qt::AlignVCenter|Qt::AlignLeft, teamName);
  painter->restore();
}

//----------------------------------------------------------------------------

QSize PairItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  PlayerPair pp = Tournament::getPlayerMngr()->getPlayerPair(index.data(Qt::UserRole).toInt());
  QString playerName = pp.getDisplayName();
  QString teamName = pp.getDisplayName_Team();
  
  int width = qMax(fntMetrics.width(playerName), fntMetrics.width(teamName));
  width += 2 * PAIR_ITEM_MARGIN;
  
  return QSize(width, PAIR_ITEM_ROW_HEIGHT);
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
    


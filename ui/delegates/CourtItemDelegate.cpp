/* 
 * File:   CourtItemDelegate.cpp
 * Author: volker
 * 
 * Created on September 7, 2014, 1:26 PM
 */

#include "CourtItemDelegate.h"

#include <QPainter>

#include "Tournament.h"
#include "Match.h"
#include "DelegateItemLED.h"
#include "ui/GuiHelpers.h"


using namespace QTournament;

CourtItemDelegate::CourtItemDelegate(QObject* parent)
: QStyledItemDelegate(parent), proxy(nullptr), fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

void CourtItemDelegate::setProxy(QAbstractProxyModel *_proxy)
{
  proxy = _proxy;
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  if (proxy)
  {
    row = (proxy->mapToSource(index)).row();
  }
  auto co = Tournament::getCourtMngr()->getCourtBySeqNum(row);
  auto ma = co->getMatch();
  
  // Fill the cell with the selection color, if necessary
  bool isItemSelected = false;
  if(option.state & QStyle::State_Selected)
  {
    QColor bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);
    isItemSelected = true;
  }
  
  // paint logic for the second column, the match description (if any match is assigned)
  if (index.column() == 1)
  {
    // if no match is assigned then display a status message and return
    if (ma == nullptr)
    {
      QString label;
      OBJ_STATE stat = co->getState();
      bool manual = co->isManualAssignmentOnly();

      if ((stat == STAT_CO_AVAIL) && !manual)
      {
        label = tr("(free)");
      }
      else if ((stat == STAT_CO_AVAIL) && manual)
      {
        label = tr("(free, manual match assignment only)");
      }
      else if (stat == STAT_CO_DISABLED)
      {
        label = tr("(disabled)");
      }
      QColor txtCol = isItemSelected ? QColor(Qt::white) : QColor(Qt::darkGray);
      GuiHelpers::drawFormattedText(painter, option.rect, label, Qt::AlignVCenter | Qt::AlignCenter, false, true, QFont(), txtCol);
      //painter->drawText(option.rect, Qt::AlignCenter, label);
      return;
    }

    // Okay, we do have a match assigned to this court
    // so pick a paint method for the match info,
    // depending on whether the cell is selected or not
    if(isItemSelected)
    {
      paintMatchInfoCell_Selected(painter, option, *ma);
    } else {
      paintMatchInfoCell_Unselected(painter, option, *ma);
    }

  } else {
    // for any other column just draw the plain text content
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
 
  
}

//----------------------------------------------------------------------------

QSize CourtItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
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
  int height = (row == selectedRow) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  return QSize(width, height);
}

//----------------------------------------------------------------------------

void CourtItemDelegate::setSelectedRow(int _selRow)
{
  selectedRow = _selRow;
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paintMatchInfoCell_Selected(QPainter *painter, const QStyleOptionViewItem &option, const Match &ma) const
{
  QRect r = option.rect;

  // draw a status indicator ("LED light")
  //DelegateItemLED{}(painter, r, ITEM_MARGIN, ITEM_STAT_INDICATOR_SIZE, ma->getState(), Qt::blue);

  // split the available space up into three rows of text
  int vertMargin = (ITEM_ROW_HEIGHT_SELECTED - 3 * ITEM_TEXT_ROW_HEIGHT - 2 * ITEM_TEXT_ROW_SKIP) / 2;
  // adjust the width to margin on the left and right. Move down to center all rows vertically in the cell.
  QRect row1 = r.adjusted(2 * ITEM_MARGIN, vertMargin, -2 * ITEM_MARGIN, 0);
  row1.setHeight(ITEM_TEXT_ROW_HEIGHT);
  QRect row2 = row1.translated(0, ITEM_TEXT_ROW_HEIGHT + ITEM_TEXT_ROW_SKIP);
  QRect row3 = row2.translated(0, ITEM_TEXT_ROW_HEIGHT + ITEM_TEXT_ROW_SKIP);

  // draw the player names
  GuiHelpers::drawFormattedText(painter, row1, ma.getDisplayName(), Qt::AlignVCenter|Qt::AlignLeft, true, false, QFont(), QColor(Qt::white), 1.2);

  // draw the first info line with the match number and the category
  QString txt = tr("Match number:");
  txt += " " + QString::number(ma.getMatchNumber()) + "    ";
  txt += tr("Category:");
  txt += " " + ma.getCategory().getName();
  GuiHelpers::drawFormattedText(painter, row2, txt, Qt::AlignVCenter|Qt::AlignLeft, false, false, 1.0);

  // draw a second info line with call times
  txt = "1. call: XXXXXXXX,   2. call: XXXXXXXXXX,    3. call: XXXXXXXXX";
  GuiHelpers::drawFormattedText(painter, row3, txt, Qt::AlignVCenter|Qt::AlignLeft, false, false, 1.0);
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paintMatchInfoCell_Unselected(QPainter *painter, const QStyleOptionViewItem &option, const Match &ma) const
{
  QRect r = option.rect;

  // draw a status indicator ("LED light")
  //DelegateItemLED{}(painter, r, ITEM_MARGIN, ITEM_STAT_INDICATOR_SIZE, ma->getState(), Qt::blue);

  // draw the name
  r.adjust(2 * ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
  QString txt = ma.getDisplayName();
  painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
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
    


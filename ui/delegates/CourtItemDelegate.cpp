/* 
 * File:   CourtItemDelegate.cpp
 * Author: volker
 * 
 * Created on September 7, 2014, 1:26 PM
 */

#include "CourtItemDelegate.h"

#include <QPainter>

#include <QDateTime>

#include "Match.h"
#include "DelegateItemLED.h"
#include "ui/GuiHelpers.h"
#include "CourtMngr.h"
#include "MatchGroup.h"

using namespace QTournament;

CourtItemDelegate::CourtItemDelegate(TournamentDB* _db, QObject* parent)
: QStyledItemDelegate(parent), db(_db), proxy(nullptr), normalFont(QFont()),
  fntMetrics(QFontMetricsF(normalFont)),
  fntMetrics_Large(fntMetrics)  // this a dummy value only
{
  largeFont = QFont();
  largeFont.setPointSizeF(largeFont.pointSizeF() * LARGE_TEXT_SIZE_FAC);
  fntMetrics_Large = QFontMetricsF(largeFont);
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
  CourtMngr cm{db};
  auto co = cm.getCourtBySeqNum(row);
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
    if (isItemSelected)
    {
      GuiHelpers::drawFormattedText(painter, option.rect, index.data(Qt::DisplayRole).toString(),
                                    Qt::AlignVCenter|Qt::AlignCenter, true, false, normalFont, QColor(Qt::white), 1.0);
    } else {
      painter->drawText(option.rect, Qt::AlignVCenter|Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
    }
  }
 
  
}

//----------------------------------------------------------------------------

QSize CourtItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * ITEM_MARGIN;   // this is actually a wrong value for doubles matches, but it's worst case estimate
  
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
  // do we need one or two rows for the player names?
  bool isDoubles = ma.getPlayerPair1().hasPlayer2();

  // calc the vertical margin
  // because we want the content to be vertically centered in the cell,
  // the vertical margin depends on the match type
  //

  // start with the height of the info rows
  double infoBlockHeight = (3 + 2 * ITEM_TEXT_ROW_SKIP_PERC) * fntMetrics.height();

  // now estimate the height of the player name block
  //
  // include 2 * ITEM_MARGIN as some extra space between the player names
  // and the info block
  double playerNameHeight = (1 + ITEM_TEXT_ROW_SKIP_PERC) * fntMetrics_Large.height();
  if (isDoubles) playerNameHeight *= 2;
  playerNameHeight += 2 * ITEM_MARGIN;

  // put everything together and calculate the margin
  double totalTextHeight = playerNameHeight + infoBlockHeight;
  double vertMargin = (ITEM_ROW_HEIGHT_SELECTED - totalTextHeight) / 2.0;

  // draw the player names
  QRect r = option.rect;
  int x0 = r.x() + ITEM_MARGIN;
  int y0 = r.y() + vertMargin;
  QRectF playerRect(r.x(), r.y() + vertMargin, r.width(), playerNameHeight - 2 * ITEM_MARGIN);
  GuiHelpers::drawTwoLinePlayerPairNames_Centered(painter, playerRect, ma, "", "", ITEM_TEXT_ROW_SKIP_PERC, true, false, largeFont, QColor(Qt::yellow));

  // draw the first info line with the match number and the category
  QString txt = tr("Match number:");
  txt += " " + QString::number(ma.getMatchNumber()) + "    ";
  txt += tr("Category:");
  txt += " " + ma.getCategory().getName() + "    ";
  txt += tr("Round: ") + QString::number(ma.getMatchGroup().getRound());
  double baseline = y0 + playerNameHeight + fntMetrics.ascent();
  GuiHelpers::drawFormattedText(painter, x0, baseline, txt, true, false, normalFont, QColor(Qt::white));

  // draw a second info line with call times
  QDateTime startTime = ma.getStartTime();
  assert(startTime.isValid());
  txt = tr("Start time: ") + startTime.toString("HH:mm");

  auto callTimeList = ma.getAdditionalCallTimes();
  if (!(callTimeList.isEmpty()))
  {
    txt += ", " + tr("additional calls at ");
    for (QDateTime call : callTimeList)
    {
      txt += call.toString("HH:mm") + ", ";
    }
    txt.chop(2);
  }
  baseline += fntMetrics.height() * (1 + ITEM_TEXT_ROW_SKIP_PERC);
  GuiHelpers::drawFormattedText(painter, x0, baseline, txt, true, false, normalFont, QColor(Qt::white));

  // draw the third info line with umpire information
  txt = tr("Umpire: ");
  REFEREE_MODE refMode = ma.get_RAW_RefereeMode();
  if (refMode == REFEREE_MODE::NONE)
  {
    txt += tr("none");
  }
  else if (refMode == REFEREE_MODE::HANDWRITTEN)
  {
    txt += tr("manually assigned");
  }
  else
  {
    upPlayer referee = ma.getAssignedReferee();
    assert(referee != nullptr);
    txt += referee->getDisplayName_FirstNameFirst();
  }
  baseline += fntMetrics.height() * (1 + ITEM_TEXT_ROW_SKIP_PERC);
  GuiHelpers::drawFormattedText(painter, x0, baseline, txt, true, false, normalFont, QColor(Qt::white));
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paintMatchInfoCell_Unselected(QPainter *painter, const QStyleOptionViewItem &option, const Match &ma) const
{
  // calculate the position of the text baseline
  double vertMargin = (ITEM_ROW_HEIGHT - fntMetrics.height()) / 2.0;
  double yBaseline = option.rect.y() + vertMargin + fntMetrics.ascent();

  // draw a simple, single line match info
  GuiHelpers::drawFormattedText(painter, option.rect.x() + ITEM_MARGIN, yBaseline,
                                ma.getDisplayName("", ""));
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
    


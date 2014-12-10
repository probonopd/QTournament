
#include "DelegateItemLED.h"

const QMap<OBJ_STATE, QColor> DelegateItemLED::state2color = {
  {STAT_CAT_FROZEN, Qt::darkYellow},
  {STAT_CAT_IDLE, Qt::darkGreen},
  {STAT_CAT_PLAYING, Qt::green},
  {STAT_CAT_FINALIZED, Qt::darkRed},
  {STAT_PL_IDLE, Qt::green},
//  {STAT_, Qt::},
//  {STAT_, Qt::},
//  {STAT_, Qt::},
//  {STAT_, Qt::},
//  {STAT_, Qt::},
};

DelegateItemLED::DelegateItemLED()
{
}

void DelegateItemLED::operator ()(QPainter* painter, QRect itemArea, int margin, int size, OBJ_STATE state, QColor defaultColor)
{
  if (state2color.keys().contains(state))
  {
    this->operator ()(painter, itemArea, margin, size, state2color[state]);
  } else {
    this->operator ()(painter, itemArea, margin, size, defaultColor);
  }
}

void DelegateItemLED::operator ()(QPainter* painter, QRect itemArea, int margin, int size, QColor fillColor)
{
  QRect indicatorBox = itemArea;
  indicatorBox.adjust(margin, 0, 0, 0);
  int topY = itemArea.center().y() - size / 2;
  indicatorBox.setTop(topY);
  indicatorBox.setWidth(size);
  indicatorBox.setHeight(size);
  painter->save();
  painter->setBrush(QBrush(fillColor));
  QPen zeroPen = QPen();
  zeroPen.setStyle(Qt::NoPen);
  painter->setPen(zeroPen);
  painter->drawEllipse(indicatorBox);
  painter->restore();

}

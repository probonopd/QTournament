#ifndef DELEGATEITEMLED_H
#define DELEGATEITEMLED_H

#include <QPainter>
#include <QStyledItemDelegate>
#include <QMap>
#include "TournamentDataDefs.h"

/**
 * @brief A functor for painting a "LED indicator" (a circle with a status color) in a delegate's paint area
 */

using namespace QTournament;

class DelegateItemLED
{
public:
  DelegateItemLED();

  void operator()(QPainter* painter, QRect itemArea, int margin, int size, QColor fillColor);
  void operator()(QPainter* painter, QRect itemArea, int margin, int size, OBJ_STATE state, QColor defaultColor=Qt::white);

  const static QMap<OBJ_STATE, QColor> state2color;
};

#endif // DELEGATEITEMLED_H

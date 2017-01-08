/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>

#include <QHeaderView>
#include <QScrollBar>
#include <QResizeEvent>

#include "AutoSizingTable.h"

namespace GuiHelpers
{

  bool ColumnAutoSizer::setRubberBandCol(int colId)
  {
    if ((colId >= 0) && (colId < colList.size()))
    {
      rubberBandCol = colId;
      return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------

  vector<int> ColumnAutoSizer::getColWidths(int useableWidth) const
  {
    // sum up all relative column widths
    int totalWidthUnits = 0;
    for (const AutosizeColumnDescr& cd : colList)
    {
      totalWidthUnits += cd.relWidth;
    }

    // determine the "physical" width increment per relative unit
    double widthUnit = (totalWidthUnits * 1.0) / (useableWidth * 1.0);

    // prepare the result vector
    vector<int> result;
    double usedWidth = 0;
    int usedWidth_int = 0;

    // calc the widths for each column
    for (const AutosizeColumnDescr& cd : colList)
    {
      // this ugly procedure should distribute rounding
      // errors more evenly over the columns, especially
      // if we have many columns with the same relative
      // width
      double oldWidth = usedWidth;
      usedWidth += cd.relWidth * widthUnit;
      double w_ = usedWidth - oldWidth;

      int w = round(w_);
      usedWidth_int += w;
      result.push_back(w);
    }

    // adjust the total column width to exactly fit the
    // useableWidth
    if (rubberBandCol < 0)
    {
      // default: adjust the last column to account for
      // overall rounding errors
      int widthError = usedWidth_int - useableWidth;
      if (widthError != 0)
      {
        int lastColWidth = result.back();
        lastColWidth -= widthError;
        result.pop_back();
        result.push_back(lastColWidth);
      }
    } else {
      // if we have a rubberband column, apply min/max
      // column widths and assign the remaining width
      // to the rubberband column
      usedWidth_int = 0;
      int idx = 0;
      while (idx < colList.size())
      {
        if (idx != rubberBandCol)
        {
          const AutosizeColumnDescr& cd = colList[idx];

          int w = result[idx];
          if (w < cd.absMinWidth) w = cd.absMinWidth;
          if (w > cd.absMaxWidth) w = cd.absMaxWidth;
          result[idx] = w;
          usedWidth_int += w;
        }
        ++idx;
      }
      result[rubberBandCol] = useableWidth - usedWidth_int;
    }

    return result;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  AutoSizingTableWidget::AutoSizingTableWidget(const AutosizeColumnDescrList& colDescr, QWidget* parent)
    :QTableWidget{parent}, ColumnAutoSizer{colDescr}, defaultDelegate{nullptr}, customDelegate{nullptr}
  {
    // set the column headers
    QStringList hdr;
    for (const AutosizeColumnDescr& cd : colList)
    {
      hdr.push_back(cd.colName);
    }
    setColumnCount(hdr.length());
    setHorizontalHeaderLabels(hdr);
    verticalHeader()->hide();

    // disable the horizontal scrollbar
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // set selection mode to "row" and "single"
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // store the default delegate for later
    defaultDelegate = itemDelegate();
  }

  //----------------------------------------------------------------------------

  AutoSizingTableWidget::~AutoSizingTableWidget()
  {
    if (defaultDelegate != nullptr) delete defaultDelegate;
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::setCustomDelegate(QAbstractItemDelegate* custDelegate)
  {
    if (custDelegate == nullptr)
    {
      restoreDefaultDelegate();
      return;
    }

    customDelegate.reset(custDelegate);
    setItemDelegate(customDelegate.get());
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::restoreDefaultDelegate()
  {
    setItemDelegate(defaultDelegate);
    customDelegate.reset();
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::resizeEvent(QResizeEvent* _event)
  {
    // call parent handler
    QTableWidget::resizeEvent(_event);

    // resize all columns
    int widthAvail = width();
    if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
    {
      widthAvail -= verticalScrollBar()->width();
    }
    auto colWidths = getColWidths(widthAvail);
    int idx = 0;
    while (idx < colWidths.size())
    {
      setColumnWidth(idx, colWidths[idx]);
    }

    // finish event processing
    _event->accept();
  }


}

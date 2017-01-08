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

#ifndef AUTOSIZINGTABLE_H
#define AUTOSIZINGTABLE_H

#include <vector>
#include <memory>

#include <QString>
#include <QTableWidget>
#include <QResizeEvent>
#include <QScrollBar>

#include "TournamentDB.h"

using namespace std;

namespace GuiHelpers
{
  struct AutosizeColumnDescr
  {
    QString colName;
    int relWidth;
    int absMinWidth = -1;
    int absMaxWidth = -1;

    AutosizeColumnDescr(const QString& cName, int rw, int minW = -1, int maxW = -1)
      :colName{cName}, relWidth{rw}, absMinWidth{minW}, absMaxWidth{maxW} {}
  };
  using AutosizeColumnDescrList = vector<AutosizeColumnDescr>;

  //----------------------------------------------------------------------------

  class ColumnAutoSizer
  {
  public:
    explicit ColumnAutoSizer(const AutosizeColumnDescrList& colDescr)
      :colList{colDescr}, rubberBandCol{-1} {}
    virtual ~ColumnAutoSizer(){}

    bool setRubberBandCol(int colId);

  protected:
    AutosizeColumnDescrList colList;
    int rubberBandCol;

    vector<int> getColWidths(int useableWidth) const;
  };

  //----------------------------------------------------------------------------

  template<typename TableTypeName>
  class AutoSizingTable : public TableTypeName, public ColumnAutoSizer
  {
  public:
    explicit AutoSizingTable(const AutosizeColumnDescrList& colDescr, QWidget *parent = 0)
      :QTableWidget{parent}, ColumnAutoSizer{colDescr}, defaultDelegate{nullptr}, customDelegate{nullptr}
    {
      // hide row numbers
      TableTypeName::verticalHeader()->hide();

      // disable the horizontal scrollbar
      TableTypeName::setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

      // set selection mode to "row" and "single"
      TableTypeName::setSelectionMode(QAbstractItemView::SingleSelection);
      TableTypeName::setSelectionBehavior(QAbstractItemView::SelectRows);

      // store the default delegate for later
      defaultDelegate = TableTypeName::itemDelegate();
    }

    //----------------------------------------------------------------------------

    virtual ~AutoSizingTable()
    {
      if (defaultDelegate != nullptr) delete defaultDelegate;
    }

    //----------------------------------------------------------------------------

    void setCustomDelegate(QAbstractItemDelegate* custDelegate)
    {
      if (custDelegate == nullptr)
      {
        restoreDefaultDelegate();
        return;
      }

      customDelegate.reset(custDelegate);
      TableTypeName::setItemDelegate(customDelegate.get());
    }

    //----------------------------------------------------------------------------

    void restoreDefaultDelegate()
    {
      TableTypeName::setItemDelegate(defaultDelegate);
      customDelegate.reset();
    }

    //----------------------------------------------------------------------------

    void autosizeColumns()
    {
      int widthAvail = TableTypeName::width();
      if ((TableTypeName::verticalScrollBar() != nullptr) && (TableTypeName::verticalScrollBar()->isVisible()))
      {
        widthAvail -= TableTypeName::verticalScrollBar()->width();
      }
      auto colWidths = getColWidths(widthAvail);
      int idx = 0;
      while (idx < colWidths.size())
      {
        TableTypeName::setColumnWidth(idx, colWidths[idx]);
        ++idx;
      }
    }

  protected:
    QAbstractItemDelegate* defaultDelegate;
    unique_ptr<QAbstractItemDelegate> customDelegate;

    virtual void resizeEvent(QResizeEvent* _event) override
    {
      // call parent handler
      TableTypeName::resizeEvent(_event);

      // resize all columns
      autosizeColumns();

      // finish event processing
      _event->accept();
    }

    //----------------------------------------------------------------------------

  };

  //----------------------------------------------------------------------------

  class AutoSizingTableWidget : public AutoSizingTable<QTableWidget>
  {
    Q_OBJECT

  public:
    explicit AutoSizingTableWidget(const AutosizeColumnDescrList& colDescr, QWidget *parent = 0);
    virtual ~AutoSizingTableWidget() {}
  };

  //----------------------------------------------------------------------------

  class AutoSizingTableWidget_WithDatabase : public AutoSizingTableWidget
  {
    Q_OBJECT

  public:
    explicit AutoSizingTableWidget_WithDatabase(const AutosizeColumnDescrList& colDescr, QWidget *parent = 0);
    virtual ~AutoSizingTableWidget_WithDatabase() {}

    void setDatabase(QTournament::TournamentDB* _db);

  protected:
    QTournament::TournamentDB* db;

    virtual void hook_onTournamentOpened() {}
    virtual void hook_onTournamentClosed() {}
  };
}
#endif // AUTOSIZINGTABLE_H

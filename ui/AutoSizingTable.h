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

  class AutoSizingTableWidget : public QTableWidget, public ColumnAutoSizer
  {
    Q_OBJECT

  public:
    explicit AutoSizingTableWidget(const AutosizeColumnDescrList& colDescr, QWidget *parent = 0);
    virtual ~AutoSizingTableWidget();

    void setCustomDelegate(QAbstractItemDelegate* custDelegate);  // TAKES OWNERSHIP!!
    void restoreDefaultDelegate();
    void autosizeColumns();

  protected:
    QAbstractItemDelegate* defaultDelegate;
    unique_ptr<QAbstractItemDelegate> customDelegate;

    virtual void resizeEvent(QResizeEvent *_event) override;
  };
}
#endif // AUTOSIZINGTABLE_H

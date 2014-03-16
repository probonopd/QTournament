/* 
 * File:   TeamListView.cpp
 * Author: volker
 * 
 * Created on March 15, 2014, 7:28 PM
 */

#include <qt/QtWidgets/qwidget.h>
#include <qt/QtWidgets/qmessagebox.h>

#include "TeamListView.h"

TeamListView::TeamListView(QWidget* parent)
:QListView(parent)
{
  // an empty model for clearing the list when
  // no tournament is open
  emptyModel = new QStringListModel();
}

TeamListView::~TeamListView()
{
  delete emptyModel;
}

void TeamListView::onTournamentClosed()
{
  tnmt = 0;
  setModel(emptyModel);
  setEnabled(false);
}

void TeamListView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  setModel(Tournament::getTeamListModel());
  setEnabled(true);
}
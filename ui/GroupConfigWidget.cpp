/*
 * File:   GroupConfigWidget.cpp
 * Author: volker
 *
 * Created on August 16, 2014, 5:42 PM
 */

#include "GroupConfigWidget.h"

GroupConfigWidget::GroupConfigWidget()
{
  widget.setupUi(this);
}

GroupConfigWidget::~GroupConfigWidget()
{
}

GroupConfigWidget::GroupConfigWidget(QWidget* parent) : QWidget(parent)
{
  widget.setupUi(this);
}

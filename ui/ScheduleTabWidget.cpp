#include "ScheduleTabWidget.h"
#include "ui_ScheduleTabWidget.h"

ScheduleTabWidget::ScheduleTabWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScheduleTabWidget)
{
    ui->setupUi(this);
}

ScheduleTabWidget::~ScheduleTabWidget()
{
    delete ui;
}

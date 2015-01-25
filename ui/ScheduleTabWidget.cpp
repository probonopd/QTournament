#include "ScheduleTabWidget.h"
#include "ui_ScheduleTabWidget.h"

ScheduleTabWidget::ScheduleTabWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScheduleTabWidget)
{
    ui->setupUi(this);

    ui->mgIdleView->setFilter(MatchGroupTableView::FilterType::IDLE);
    ui->mgStagedView->setFilter(MatchGroupTableView::FilterType::STAGED);
}

//----------------------------------------------------------------------------

ScheduleTabWidget::~ScheduleTabWidget()
{
    delete ui;
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnStageClicked()
{
  ui->mgIdleView->setFilter(MatchGroupTableView::FilterType::IDLE);
  ui->mgStagedView->setFilter(MatchGroupTableView::FilterType::STAGED);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnUnstageClicked()
{
  ui->mgIdleView->setFilter(MatchGroupTableView::FilterType::NONE);
  ui->mgStagedView->setFilter(MatchGroupTableView::FilterType::NONE);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnScheduleClicked()
{

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


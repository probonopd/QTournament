#ifndef SCHEDULETABWIDGET_H
#define SCHEDULETABWIDGET_H

#include <QDialog>

namespace Ui {
class ScheduleTabWidget;
}

class ScheduleTabWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleTabWidget(QWidget *parent = 0);
    ~ScheduleTabWidget();

private:
    Ui::ScheduleTabWidget *ui;
};

#endif // SCHEDULETABWIDGET_H

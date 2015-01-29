#ifndef SCHEDULETABWIDGET_H
#define SCHEDULETABWIDGET_H

#include <QDialog>
#include <QItemSelection>
#include <QItemSelectionModel>

namespace Ui {
class ScheduleTabWidget;
}

class ScheduleTabWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleTabWidget(QWidget *parent = 0);
    ~ScheduleTabWidget();

public slots:
    void onBtnStageClicked();
    void onBtnUnstageClicked();
    void onBtnScheduleClicked();
    void onIdleSelectionChanged(const QItemSelection &, const QItemSelection &);
    void onStagedSelectionChanged(const QItemSelection &, const QItemSelection &);

private:
    Ui::ScheduleTabWidget *ui;
    void updateButtons();
};

#endif // SCHEDULETABWIDGET_H

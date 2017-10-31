#ifndef DLGCONNECTIONSETTINGS_H
#define DLGCONNECTIONSETTINGS_H

#include <QDialog>

namespace Ui {
  class DlgConnectionSettings;
}

class DlgConnectionSettings : public QDialog
{
  Q_OBJECT

public:
  explicit DlgConnectionSettings(QWidget *parent, QString customUrl, QString customPubKey, int customTimeout_ms);
  ~DlgConnectionSettings();

  QString getCustomUrl() const;
  QString getCustomPubKey() const;
  int getCustomTimeout_ms() const;

protected:
  bool validateInputs();

protected slots:
  void onCheckboxToggled();
  void onBtnOkayClicked();

private:
  Ui::DlgConnectionSettings *ui;
};

#endif // DLGCONNECTIONSETTINGS_H

#ifndef DLGPLAYERPROFILE_H
#define DLGPLAYERPROFILE_H

#include <QDialog>

namespace Ui {
  class DlgPlayerProfile;
}

class DlgPlayerProfile : public QDialog
{
  Q_OBJECT

public:
  explicit DlgPlayerProfile(QWidget *parent = 0);
  ~DlgPlayerProfile();

private:
  Ui::DlgPlayerProfile *ui;
};

#endif // DLGPLAYERPROFILE_H

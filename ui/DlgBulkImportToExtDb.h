#ifndef DLGBULKIMPORTTOEXTDB_H
#define DLGBULKIMPORTTOEXTDB_H

#include <memory>

#include <QDialog>
#include <QTextDocument>

namespace Ui {
  class DlgBulkImportToExtDb;
}

using namespace std;

class DlgBulkImportToExtDb : public QDialog
{
  Q_OBJECT

public:
  explicit DlgBulkImportToExtDb(QWidget *parent = 0);
  ~DlgBulkImportToExtDb();
  QString getText() const;

private:
  Ui::DlgBulkImportToExtDb *ui;
  unique_ptr<QTextDocument> doc;
};

#endif // DLGBULKIMPORTTOEXTDB_H

#include <memory>

#include <QTextDocument>

#include "DlgBulkImportToExtDb.h"
#include "ui_DlgBulkImportToExtDb.h"

DlgBulkImportToExtDb::DlgBulkImportToExtDb(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgBulkImportToExtDb)
{
  ui->setupUi(this);

  // prepare a new text document and assign
  // it to the text edit widget
  //doc = make_unique<QTextDocument>(this);
  //ui->textEdit->setDocument(doc.get());  // the text edit doesn't take ownership
}

DlgBulkImportToExtDb::~DlgBulkImportToExtDb()
{
  delete ui;
}

QString DlgBulkImportToExtDb::getText() const
{
  return ui->textEdit->document()->toPlainText();
}

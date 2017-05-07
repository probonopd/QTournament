#include <algorithm>

#include <QDialogButtonBox>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QRadioButton>

#include "DlgImportCSV_Step1.h"
#include "ui_DlgImportCSV_Step1.h"

#include "TournamentDB.h"
#include "CatMngr.h"
#include "CSVImporter.h"

using namespace QTournament;

DlgImportCSV_Step1::DlgImportCSV_Step1(QWidget *parent, QTournament::TournamentDB* _db) :
  QDialog(parent),
  ui(new Ui::DlgImportCSV_Step1), db{_db}
{
  ui->setupUi(this);

  // fill the combo box with category names;
  // use only categories that are still open for new players
  ui->cbCat->addItem(tr("<Please select>"), -1);
  CatMngr cm{db};
  auto allCats = cm.getAllCategories();
  std::sort(allCats.begin(), allCats.end(), [](const Category& c1, const Category& c2)
  {
    return (c1.getName() < c2.getName());
  });
  for (const Category& c : cm.getAllCategories())
  {
    if (c.canAddPlayers())
    {
      ui->cbCat->addItem(c.getName(), c.getId());
    }
  }

  // disable the Next-button as long as the text box is empty
  ui->btnNext->setEnabled(false);
}

//----------------------------------------------------------------------------

DlgImportCSV_Step1::~DlgImportCSV_Step1()
{
  delete ui;
}

//----------------------------------------------------------------------------

vector<vector<string>> DlgImportCSV_Step1::getSplitData() const
{
  vector<vector<string>> result;
  QString plain = ui->txtBox->document()->toPlainText();
  string raw{plain.toUtf8().constData()};

  // is an additional category selected
  string extraCat;
  int extraCatId = ui->cbCat->currentData().toInt();
  if (extraCatId > 0)
  {
    extraCat = string{ui->cbCat->currentText().toUtf8().constData()};
  }

  // determine the delimiter character
  string delim = ",";
  if (ui->rbSemi->isChecked())
  {
    delim = ";";
  }
  if (ui->rbTab->isChecked())
  {
    delim = "\t";
  }

  // return the split result
  return splitCSV(raw, delim, extraCat);
}

//----------------------------------------------------------------------------

void DlgImportCSV_Step1::onContentChanged()
{
  ui->btnNext->setEnabled(!(ui->txtBox->document()->isEmpty()));
}

//----------------------------------------------------------------------------


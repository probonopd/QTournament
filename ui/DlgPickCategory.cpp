#include <QListWidgetItem>

#include "DlgPickCategory.h"
#include "ui_DlgPickCategory.h"
#include "CatMngr.h"

using namespace QTournament;

DlgPickCategory::DlgPickCategory(QWidget *parent, QTournament::TournamentDB* _db,
                                 QTournament::Sex sex) :
  QDialog(parent),
  ui(new Ui::DlgPickCategory), db{_db}
{
  ui->setupUi(this);

  // build a list of available categories
  std::vector<Category> availCats;
  CatMngr cm{db};
  for (const Category& cat : cm.getAllCategories())
  {
    if (cat.canAddPlayers())
    {
      if (sex != Sex::DontCare)
      {
        if (cat.getAddState(sex) != CatAddState::CanJoin) continue;
      }

      availCats.push_back(cat);
    }
  }
  std::sort(availCats.begin(), availCats.end(), [](const Category& c1, const Category& c2)
  {
    return (c1.getName() < c2.getName());
  });

  // add the categories to the list
  for (const Category& cat : availCats)
  {
    QListWidgetItem* newItem = new QListWidgetItem(cat.getName());
    newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    newItem->setData(Qt::UserRole, cat.getId());

    ui->catList->addItem(newItem);
  }
}

//----------------------------------------------------------------------------

void DlgPickCategory::applyPreselection(const std::vector<Category>& preSelection)
{
  CatMngr cm{db};

  for (int row = 0; row < ui->catList->count(); ++row)
  {
    QListWidgetItem* it = ui->catList->item(row);
    int catId = it->data(Qt::UserRole).toInt();
    Category cat = cm.getCategoryById(catId);

    if (Sloppy::isInVector<Category>(preSelection, cat))
    {
      it->setCheckState(Qt::Checked);
    } else {
      it->setCheckState(Qt::Unchecked);
    }
  }
}

//----------------------------------------------------------------------------

void DlgPickCategory::applyPreselection(const QString& commaSepCatNames)
{
  CatMngr cm{db};
  std::vector<Category> catSelection;

  for (const QString& catName : commaSepCatNames.split(","))
  {
    QString cn = catName.trimmed();
    if (cm.hasCategory(cn))
    {
      catSelection.push_back(cm.getCategory(cn));
    }
  }

  applyPreselection(catSelection);
}

//----------------------------------------------------------------------------

QString DlgPickCategory::getSelection_CommaSep() const
{
  QString result;
  std::vector<Category> selection = getSelection();
  for (const Category& cat : selection)
  {
    result += cat.getName() + ", ";
  }
  if (!(result.isEmpty()))
  {
    result.chop(2);
  }

  return result;
}

//----------------------------------------------------------------------------

vector<int> DlgPickCategory::getSelection_Id() const
{
  std::vector<int> result;

  for (int row = 0; row < ui->catList->count(); ++row)
  {
    QListWidgetItem* it = ui->catList->item(row);
    if (it->checkState() != Qt::Checked) continue;

    int catId = it->data(Qt::UserRole).toInt();
    result.push_back(catId);
  }

  return result;
}

//----------------------------------------------------------------------------

vector<QString> DlgPickCategory::getSelection_strVec() const
{
  CatMngr cm{db};
  std::vector<QString> result;

  for (int row = 0; row < ui->catList->count(); ++row)
  {
    QListWidgetItem* it = ui->catList->item(row);
    if (it->checkState() != Qt::Checked) continue;

    int catId = it->data(Qt::UserRole).toInt();
    Category cat = cm.getCategoryById(catId);
    result.push_back(cat.getName());
  }

  return result;
}

//----------------------------------------------------------------------------

vector<Category> DlgPickCategory::getSelection() const
{
  CatMngr cm{db};
  std::vector<Category> result;

  for (int row = 0; row < ui->catList->count(); ++row)
  {
    QListWidgetItem* it = ui->catList->item(row);
    if (it->checkState() != Qt::Checked) continue;

    int catId = it->data(Qt::UserRole).toInt();
    result.push_back(cm.getCategoryById(catId));
  }

  return result;
}

//----------------------------------------------------------------------------

DlgPickCategory::~DlgPickCategory()
{
  delete ui;
}

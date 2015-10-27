#include <memory>

#include "Tournament.h"
#include "BracketVisData.h"

unique_ptr<QTournament::BracketVisData> QTournament::BracketVisData::getExisting(const QTournament::Category& _cat)
{
  // acquire a database handle
  TournamentDB* _db = Tournament::getDatabaseHandle();

  // check if the requested category has visualization data
  DbTab catTab = _db->getTab(TAB_CATEGORY);
  QVariant visData = catTab[_cat.getId()][CAT_BRACKET_VIS_DATA];
  if (visData.isNull())
  {
    return nullptr;
  }

  // FIX ME: check the consistency of the is data? Not for now, add later


  // the category is valid, create and return a new object
  auto result = new BracketVisData(_db, _cat);
  return unique_ptr<BracketVisData>(result);
}

//----------------------------------------------------------------------------

unique_ptr<BracketVisData> BracketVisData::createNew(const Category& _cat, BRACKET_PAGE_ORIENTATION orientation, BRACKET_LABEL_POS firstPageLabelPos)
{
  // check if the category already has visualization data and
  // return nullptr to indicate an error in this case
  auto tmp = BracketVisData::getExisting(_cat);
  if (tmp != nullptr)
  {
    return nullptr;
  }

  // create a new, empty object
  TournamentDB* _db = Tournament::getDatabaseHandle();
  auto result = new BracketVisData(_db, _cat);

  // populate the first page
  result->addPage(orientation, firstPageLabelPos);

  // return the object
  return unique_ptr<BracketVisData>(result);
}

//----------------------------------------------------------------------------

int BracketVisData::getNumPages() const
{
  QString visData = cat.getBracketVisDataString();

  if (visData.isEmpty()) return 0;

  return visData.count(":") + 1;
}

//----------------------------------------------------------------------------

tuple<BRACKET_PAGE_ORIENTATION, BRACKET_LABEL_POS> BracketVisData::getPageInfo(int idxPage) const
{
  QString visData = cat.getBracketVisDataString();
  if (idxPage >= getNumPages())
  {
    throw std::range_error("Attempt to access invalid bracket visualization page");
  }

  QString pageDef = visData.split(":")[idxPage];
  int iOrientation = pageDef.split(",")[0].toInt();
  int iLabelPos = pageDef.split(",")[1].toInt();

  return make_tuple(static_cast<BRACKET_PAGE_ORIENTATION>(iOrientation), static_cast<BRACKET_LABEL_POS>(iLabelPos));
}

//----------------------------------------------------------------------------

QTournament::BracketVisElementList BracketVisData::getVisElements(int idxPage)
{
  QVariantList qvl;
  qvl << BV_CAT_REF << cat.getId();
  if (idxPage >= 0)
  {
    qvl << BV_PAGE << idxPage;
  }
  return getObjectsByColumnValue<BracketVisElement>(visTab, qvl);
}

//----------------------------------------------------------------------------

void BracketVisData::addPage(BRACKET_PAGE_ORIENTATION pageOrientation, BRACKET_LABEL_POS labelOnPagePosition) const
{
  // convert the parameters into a comma-sep. string
  QString pageDef = QString::number(static_cast<int>(pageOrientation));
  pageDef += "," + QString::number(static_cast<int>(labelOnPagePosition));

  // get the existing page specification
  DbTab catTab = db->getTab(TAB_CATEGORY);
  int catId = cat.getId();
  QString visData = catTab[catId][CAT_BRACKET_VIS_DATA].toString();

  // treat the vis data as a colon-separated list of integers and add a new
  // element, if necessary
  if (!(visData.isEmpty()))
  {
    visData += ":";
  }
  visData += pageDef;

  // write the new string to the database
  catTab[catId].update(CAT_BRACKET_VIS_DATA, visData);
}

//----------------------------------------------------------------------------

void BracketVisData::addElement(const RawBracketVisElement& el)
{
  QVariantList qvl;

  qvl << BV_CAT_REF << cat.getId();

  qvl << BV_PAGE << el.page;
  qvl << BV_GRID_X0 << el.gridX0;
  qvl << BV_GRID_Y0 << el.gridY0;
  qvl << BV_SPAN_Y << el.ySpan;
  qvl << BV_ORIENTATION << static_cast<int>(el.orientation);
  qvl << BV_TERMINATOR << static_cast<int>(el.terminator);

  // FIX ME
  qvl << BV_INITIAL_RANK1 << 42;
  qvl << BV_INITIAL_RANK1 << 23;

  visTab.insertRow(qvl);
}

//----------------------------------------------------------------------------

BracketVisData::BracketVisData(TournamentDB* _db, const Category& _cat)
: GenericObjectManager(_db), visTab((*db)[TAB_BRACKET_VIS]), cat(_cat)
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//
// BracketVisElement
//



unique_ptr<Match> BracketVisElement::getLinkedMatch() const
{
  QVariant _matchId = row[BV_MATCH_REF];
  if (_matchId.isNull()) return nullptr;
  return Tournament::getMatchMngr()->getMatch(_matchId.toInt());
}

//----------------------------------------------------------------------------

BracketVisElement::BracketVisElement(TournamentDB* _db, int rowId)
  :GenericDatabaseObject(_db, TAB_BRACKET_VIS, rowId)
{

}

//----------------------------------------------------------------------------

BracketVisElement::BracketVisElement(TournamentDB* _db, TabRow row)
  :GenericDatabaseObject(_db, row)
{

}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

RawBracketVisElement::RawBracketVisElement(int visData[9])
{
  page = visData[0];
  gridX0 = visData[1];
  gridY0 = visData[2];
  ySpan = visData[3];

  yPageBreakSpan = visData[4];
  nextPageNum = visData[5];

  orientation = (visData[6] == -1) ? BRACKET_ORIENTATION::LEFT : BRACKET_ORIENTATION::RIGHT;

  switch (visData[7])
  {
  case 1:
    terminator = BRACKET_TERMINATOR::OUTWARDS;
    break;
  case -11:
    terminator = BRACKET_TERMINATOR::INWARDS;
    break;
  default:
    terminator = BRACKET_TERMINATOR::NONE;
  }
  terminatorOffsetY = visData[8];
}

//----------------------------------------------------------------------------

void RawBracketVisDataDef::addPage(BRACKET_PAGE_ORIENTATION orientation, BRACKET_LABEL_POS labelPos)
{
  pageOrientationList.push_back(orientation);
  labelPosList.push_back(labelPos);
}

//----------------------------------------------------------------------------

bool RawBracketVisDataDef::addElement(const RawBracketVisElement& el)
{
  // ensure that we only add bracket elements for existing pages
  if (el.page >= getNumPages()) return false;

  bracketElementList.push_back(el);
}

//----------------------------------------------------------------------------

tuple<BRACKET_PAGE_ORIENTATION, BRACKET_LABEL_POS> RawBracketVisDataDef::getPageInfo(int idxPage) const
{
  if (idxPage >= getNumPages())
  {
    throw std::range_error("Attempt to access invalid page in bracket visualization data!");
  }

  return make_tuple(pageOrientationList.at(idxPage), labelPosList.at(idxPage));
}

//----------------------------------------------------------------------------

RawBracketVisElement RawBracketVisDataDef::getElement(int idxElement) const
{
  if (idxElement >= bracketElementList.size())
  {
    throw std::range_error("Attempt to access invalid element in bracket visualization data!");
  }

  return bracketElementList.at(idxElement);
}

//----------------------------------------------------------------------------

void RawBracketVisDataDef::clear()
{
  pageOrientationList.clear();
  labelPosList.clear();
  bracketElementList.clear();
}

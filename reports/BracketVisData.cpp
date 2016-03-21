/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>

#include "ClausesAndQueries.h"

#include "Tournament.h"
#include "BracketVisData.h"

using namespace QTournament;

unique_ptr<QTournament::BracketVisData> QTournament::BracketVisData::getExisting(const QTournament::Category& _cat)
{
  // acquire a database handle
  auto tnmt = Tournament::getActiveTournament();
  TournamentDB* _db = tnmt->getDatabaseHandle();

  // check if the requested category has visualization data
  DbTab* catTab = _db->getTab(TAB_CATEGORY);
  TabRow r = catTab->operator [](_cat.getId());
  auto visData = r.getString2(CAT_BRACKET_VIS_DATA);
  if (visData->isNull())
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
  auto tnmt = Tournament::getActiveTournament();
  TournamentDB* _db = tnmt->getDatabaseHandle();
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
  WhereClause wc;
  wc.addIntCol(BV_CAT_REF, cat.getId());
  if (idxPage >= 0)
  {
    wc.addIntCol(BV_PAGE, idxPage);
  }
  return getObjectsByWhereClause<BracketVisElement>(wc);
}

//----------------------------------------------------------------------------

QTournament::upBracketVisElement BracketVisData::getVisElement(int idx) const
{
  WhereClause wc;
  wc.addIntCol(BV_CAT_REF, cat.getId());
  wc.addIntCol(BV_ELEMENT_ID, idx);

  return getSingleObjectByWhereClause<BracketVisElement>(wc);
}

//----------------------------------------------------------------------------

void BracketVisData::addPage(BRACKET_PAGE_ORIENTATION pageOrientation, BRACKET_LABEL_POS labelOnPagePosition) const
{
  // convert the parameters into a comma-sep. string
  string pageDef = to_string(static_cast<int>(pageOrientation));
  pageDef += "," + to_string(static_cast<int>(labelOnPagePosition));

  // get the existing page specification
  DbTab* catTab = db->getTab(TAB_CATEGORY);
  int catId = cat.getId();
  TabRow visDataRow = catTab->operator [](catId);
  auto _visData = visDataRow.getString2(CAT_BRACKET_VIS_DATA);

  // treat the vis data as a colon-separated list of integers and add a new
  // element, if necessary
  string visData = _visData->isNull() ? "" : (_visData->get() + ":");
  visData += pageDef;

  // write the new string to the database
  visDataRow.update(CAT_BRACKET_VIS_DATA, visData);
}

//----------------------------------------------------------------------------

void BracketVisData::addElement(int idx, const RawBracketVisElement& el)
{
  ColumnValueClause cvc;

  cvc.addIntCol(BV_CAT_REF, cat.getId());

  cvc.addIntCol(BV_PAGE, el.page);
  cvc.addIntCol(BV_GRID_X0, el.gridX0);
  cvc.addIntCol(BV_GRID_Y0, el.gridY0);
  cvc.addIntCol(BV_SPAN_Y, el.ySpan);
  cvc.addIntCol(BV_ORIENTATION, static_cast<int>(el.orientation));
  cvc.addIntCol(BV_TERMINATOR, static_cast<int>(el.terminator));
  cvc.addIntCol(BV_Y_PAGEBREAK_SPAN, el.yPageBreakSpan);
  cvc.addIntCol(BV_NEXT_PAGE_NUM, el.nextPageNum);
  cvc.addIntCol(BV_TERMINATOR_OFFSET_Y, el.terminatorOffsetY);
  cvc.addIntCol(BV_ELEMENT_ID, idx);
  cvc.addIntCol(BV_INITIAL_RANK1, el.initialRank1);
  cvc.addIntCol(BV_INITIAL_RANK2, el.initialRank2);
  cvc.addIntCol(BV_NEXT_WINNER_MATCH, el.nextMatchForWinner);
  cvc.addIntCol(BV_NEXT_LOSER_MATCH, el.nextMatchForLoser);
  cvc.addIntCol(BV_NEXT_MATCH_POS_FOR_WINNER, el.nextMatchPlayerPosForWinner);
  cvc.addIntCol(BV_NEXT_MATCH_POS_FOR_LOSER, el.nextMatchPlayerPosForLoser);
  tab->insertRow(cvc);
}

//----------------------------------------------------------------------------

/**
 * @brief Inserts player names (as PlayerPair ref) in bracket matches that do not have a corresponding "real" match
 *
 * Fills as many gaps as currentlt possible. Has to be called repeatedly as the tournamen progresses (e.g., every time
 * a bracket view / report is created).
 *
 */
void BracketVisData::fillMissingPlayerNames() const
{
  int catId = cat.getId();

  // get the seeding list once, we need it later...
  auto tnmt = Tournament::getActiveTournament();
  PlayerPairList seeding = tnmt->getCatMngr()->getSeeding(cat);

  bool hasModifications = true;
  while (hasModifications)   // repeat until we find no more changes to make
  {
    hasModifications = false;

    //
    // Check 1: fill-in names for initial matches (from seeding list)
    //
    QString where;
    where = "%1=%2 AND %3 IS NULL AND %4>0 AND %5>0 AND %6 IS NULL AND %7 IS NULL";
    where = where.arg(BV_CAT_REF);
    where = where.arg(catId);
    where = where.arg(BV_MATCH_REF);
    where = where.arg(BV_INITIAL_RANK1, BV_INITIAL_RANK2);
    where = where.arg(BV_PAIR1_REF, BV_PAIR2_REF);
    for (BracketVisElement el : getObjectsByWhereClause<BracketVisElement>(where.toUtf8().constData()))
    {
      int iniRank = el.getInitialRank1();
      if (iniRank <= seeding.size())
      {
        el.linkToPlayerPair(seeding.at(iniRank - 1), 1);
        hasModifications = true;
      }
      iniRank = el.getInitialRank2();
      if (iniRank <= seeding.size())
      {
        el.linkToPlayerPair(seeding.at(iniRank - 1), 2);
        hasModifications = true;
      }
    }

    //
    // Check 2: fill-in names for intermediate matches that lack both player names
    //
    where = "%1=%2 AND %3 IS NULL AND %4<=0 AND %5<=0 AND %6 IS NULL AND %7 IS NULL";
    where = where.arg(BV_CAT_REF);  // %1
    where = where.arg(catId);       // %2
    where = where.arg(BV_MATCH_REF);  // %3
    where = where.arg(BV_INITIAL_RANK1, BV_INITIAL_RANK2);  // %4, %5
    where = where.arg(BV_PAIR1_REF, BV_PAIR2_REF);   // %6, %7

    for (BracketVisElement el : getObjectsByWhereClause<BracketVisElement>(where.toUtf8().constData()))
    {
      // Pair 1:
      // is there any match pointing to this bracket element
      // as winner or loser?
      auto parentElem = getParentPlayerPairForElement(el, 1);
      if(parentElem != nullptr)
      {
        el.linkToPlayerPair(*parentElem, 1);
        hasModifications = true;
      }

      // Pair 2:
      // is there any match pointing to this bracket element
      // as winner or loser?
      parentElem = getParentPlayerPairForElement(el, 2);
      if(parentElem != nullptr)
      {
        el.linkToPlayerPair(*parentElem, 2);
        hasModifications = true;
      }
    }
  }
}

//----------------------------------------------------------------------------

BracketVisData::BracketVisData(TournamentDB* _db, const Category& _cat)
: TournamentDatabaseObjectManager(_db, TAB_BRACKET_VIS), cat(_cat)
{
}

unique_ptr<PlayerPair> BracketVisData::getParentPlayerPairForElement(const BracketVisElement& el, int pos) const
{
  // check parameter range
  if ((pos != 1) && (pos != 2)) return nullptr;

  // check element validity
  if (el.getCategoryId() != cat.getId()) return nullptr;

  int elemId = el.getBracketElementId();

  // search for a bracket element that uses this element as next winner / loser match
  QString where = "%1=%2 AND ((%3=%4 AND %5=%6) OR (%7=%4 AND %8=%6))";
  where = where.arg(BV_CAT_REF);                    // %1
  where = where.arg(cat.getId());               // %2
  where = where.arg(BV_NEXT_WINNER_MATCH);          // %3
  where = where.arg(elemId);                      // %4
  where = where.arg(BV_NEXT_MATCH_POS_FOR_WINNER);  // %5
  where = where.arg(pos);                           // %6
  where = where.arg(BV_NEXT_LOSER_MATCH);           // %7
  where = where.arg(BV_NEXT_MATCH_POS_FOR_LOSER);   // %8
  auto parentElem = getSingleObjectByWhereClause<BracketVisElement>(where.toUtf8().constData());

  // case 1: no parent
  if (parentElem == nullptr) return nullptr;

  // case 2: parent has a match assigned
  auto ma = parentElem->getLinkedMatch();
  if (ma != nullptr)
  {
    // case 2a: match is not finished, winner and loser are unknown
    if (ma->getState() != STAT_MA_FINISHED) return nullptr;

    // case 2b: match is finished, winner and loser are determined
    if (parentElem->getNextBracketElementForWinner() == elemId)
    {
      // case 2b.1: the winner of "parent" will go to "elem"
      assert(parentElem->getNextBracketElementPosForWinner() == pos);
      return ma->getWinner();
    }

    // case 2b.2: the loser or "parent" will go to "elem"
    assert(parentElem->getNextBracketElementForLoser() == elemId);
    assert(parentElem->getNextBracketElementPosForLoser() == pos);
    return ma->getLoser();
  }

  // case 3: no assigned match for parent, but exactly one assigned player pair
  // AND "elem" is the "winner child" of "parent"
  assert(ma == nullptr);
  bool hasFixedPair1 = parentElem->getLinkedPlayerPair(1) != nullptr;
  bool hasFixedPair2 = parentElem->getLinkedPlayerPair(2) != nullptr;
  if (hasFixedPair1 ^ hasFixedPair2)    // exclusive OR... only one pair is allowed
  {
    if (parentElem->getNextBracketElementForWinner() == elemId)
    {
      assert(parentElem->getNextBracketElementPosForWinner() == pos);
      return (hasFixedPair1) ? parentElem->getLinkedPlayerPair(1) : parentElem->getLinkedPlayerPair(2);
    }
  }

  // default: don't know or can't decide
  return nullptr;
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


//----------------------------------------------------------------------------


//
// BracketVisElement
//



unique_ptr<Match> BracketVisElement::getLinkedMatch() const
{
  auto _matchId = row.getInt2(BV_MATCH_REF);
  if (_matchId->isNull()) return nullptr;
  auto tnmt = Tournament::getActiveTournament();
  return tnmt->getMatchMngr()->getMatch(_matchId->get());
}

//----------------------------------------------------------------------------

Category BracketVisElement::getLinkedCategory() const
{
  int catId = row.getInt(BV_CAT_REF);
  auto tnmt = Tournament::getActiveTournament();
  return tnmt->getCatMngr()->getCategoryById(catId);
}

//----------------------------------------------------------------------------

unique_ptr<PlayerPair> BracketVisElement::getLinkedPlayerPair(int pos) const
{
  if ((pos != 1) && (pos != 2)) return nullptr;

  unique_ptr<ScalarQueryResult<int>> _pairId;
  if (pos == 1) _pairId = row.getInt2(BV_PAIR1_REF);
  else _pairId = row.getInt2(BV_PAIR2_REF);

  if (_pairId->isNull()) return nullptr;

  return make_unique<PlayerPair>(db, _pairId->get());
}

//----------------------------------------------------------------------------

bool BracketVisElement::linkToMatch(const Match& ma) const
{
  Category myCat = getLinkedCategory();
  if (ma.getCategory() != myCat) return false;

  row.update(BV_MATCH_REF, ma.getId());
  return true;
}

//----------------------------------------------------------------------------

bool BracketVisElement::linkToPlayerPair(const PlayerPair& pp, int pos) const
{
  if ((pos != 1) && (pos != 2)) return false;

  Category myCat = getLinkedCategory();
  auto ppCat = pp.getCategory(db);
  if (ppCat == nullptr) return false;

  if ((*ppCat) != myCat) return false;

  int pairId = pp.getPairId();
  if (pairId <= 0) return false;

  if (pos == 1)
  {
    row.update(BV_PAIR1_REF, pairId);
  } else {
    row.update(BV_PAIR2_REF, pairId);
  }

  return true;
}

//----------------------------------------------------------------------------

BracketVisElement::BracketVisElement(TournamentDB* _db, int rowId)
  :TournamentDatabaseObject(_db, TAB_BRACKET_VIS, rowId)
{

}

//----------------------------------------------------------------------------

BracketVisElement::BracketVisElement(TournamentDB* _db, TabRow row)
  :TournamentDatabaseObject(_db, row)
{

}

//----------------------------------------------------------------------------

unique_ptr<PlayerPair> BracketVisElement::getParentPlayerPair(int pos) const
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
  case -1:
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

/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <SqliteOverlay/ClausesAndQueries.h>

#include "BracketVisData.h"
#include "CatMngr.h"

using namespace QTournament;
using namespace SqliteOverlay;

std::optional<BracketVisData> QTournament::BracketVisData::getExisting(const QTournament::Category& _cat)
{
  // acquire a database handle
  const auto& _db = _cat.getDatabaseHandle();

  // check if the requested category has visualization data
  DbTab catTab{_db, TabCategory, false};
  TabRow r = catTab[_cat.getId()];
  auto visData = r.getString2(CAT_BracketVisData);
  if (!visData)
  {
    return {};
  }

  // FIX ME: check the consistency of the is data? Not for now, add later


  // the category is valid, create and return a new object
  return BracketVisData(_db, _cat);
}

//----------------------------------------------------------------------------

std::optional<BracketVisData> BracketVisData::createNew(const Category& _cat, BracketPageOrientation orientation, BracketLabelPos firstPageLabelPos)
{
  // check if the category already has visualization data and
  // return nullptr to indicate an error in this case
  auto tmp = BracketVisData::getExisting(_cat);
  if (tmp)
  {
    return {};
  }

  // create a new, empty object
  const auto& _db = _cat.getDatabaseHandle();
  BracketVisData result{_db, _cat};

  // populate the first page
  result.addPage(orientation, firstPageLabelPos);

  // return the object
  return result;
}

//----------------------------------------------------------------------------

int BracketVisData::getNumPages() const
{
  QString visData = cat.getBracketVisDataString();

  if (visData.isEmpty()) return 0;

  return visData.count(":") + 1;
}

//----------------------------------------------------------------------------

std::tuple<BracketPageOrientation, BracketLabelPos> BracketVisData::getPageInfo(int idxPage) const
{
  QString visData = cat.getBracketVisDataString();
  if (idxPage >= getNumPages())
  {
    throw std::range_error("Attempt to access invalid bracket visualization page");
  }

  QString pageDef = visData.split(":")[idxPage];
  int iOrientation = pageDef.split(",")[0].toInt();
  int iLabelPos = pageDef.split(",")[1].toInt();

  return std::make_tuple(static_cast<BracketPageOrientation>(iOrientation), static_cast<BracketLabelPos>(iLabelPos));
}

//----------------------------------------------------------------------------

QTournament::BracketVisElementList BracketVisData::getVisElements(int idxPage)
{
  WhereClause wc;
  wc.addCol(BV_CatRef, cat.getId());
  if (idxPage >= 0)
  {
    wc.addCol(BV_Page, idxPage);
  }
  return getObjectsByWhereClause<BracketVisElement>(wc);
}

//----------------------------------------------------------------------------

std::optional<BracketVisElement> BracketVisData::getVisElement(int idx) const
{
  WhereClause wc;
  wc.addCol(BV_CatRef, cat.getId());
  wc.addCol(BV_ElementId, idx);

  return getSingleObjectByWhereClause<BracketVisElement>(wc);
}

//----------------------------------------------------------------------------

void BracketVisData::addPage(BracketPageOrientation pageOrientation, BracketLabelPos labelOnPagePosition) const
{
  // convert the parameters into a comma-sep. string
  std::string pageDef = std::to_string(static_cast<int>(pageOrientation));
  pageDef += "," + std::to_string(static_cast<int>(labelOnPagePosition));

  // get the existing page specification
  DbTab catTab{db, TabCategory, false};
  int catId = cat.getId();
  TabRow visDataRow = catTab[catId];
  auto _visData = visDataRow.getString2(CAT_BracketVisData);

  // treat the vis data as a colon-separated list of integers and add a new
  // element, if necessary
  std::string visData = _visData ? (*_visData + ":") : "";
  visData += pageDef;

  // write the new string to the database
  visDataRow.update(CAT_BracketVisData, visData);
}

//----------------------------------------------------------------------------

void BracketVisData::addElement(int idx, const RawBracketVisElement& el)
{
  ColumnValueClause cvc;

  cvc.addCol(BV_CatRef, cat.getId());

  cvc.addCol(BV_Page, el.page);
  cvc.addCol(BV_GridX0, el.gridX0);
  cvc.addCol(BV_GridY0, el.gridY0);
  cvc.addCol(BV_SpanY, el.ySpan);
  cvc.addCol(BV_Orientation, static_cast<int>(el.orientation));
  cvc.addCol(BV_Terminator, static_cast<int>(el.terminator));
  cvc.addCol(BV_YPagebreakSpan, el.yPageBreakSpan);
  cvc.addCol(BV_NextPageNum, el.nextPageNum);
  cvc.addCol(BV_Terminator_OFFSET_Y, el.terminatorOffsetY);
  cvc.addCol(BV_ElementId, idx);
  cvc.addCol(BV_InitialRank1, el.initialRank1);
  cvc.addCol(BV_InitialRank2, el.initialRank2);
  cvc.addCol(BV_NextWinnerMatch, el.nextMatchForWinner);
  cvc.addCol(BV_NextLoserMatch, el.nextMatchForLoser);
  cvc.addCol(BV_NextMatchPosForWinner, el.nextMatchPlayerPosForWinner);
  cvc.addCol(BV_NextMatchPosForLoser, el.nextMatchPlayerPosForLoser);

  tab.insertRow(cvc);
}

//----------------------------------------------------------------------------

/**
 * @brief Inserts player names (as PlayerPair ref) in bracket matches that do not have a corresponding "real" match
 *
 * Fills as many gaps as currently possible. Has to be called repeatedly as the tournament progresses (e.g., every time
 * a bracket view / report is created).
 *
 */
void BracketVisData::fillMissingPlayerNames() const
{
  int catId = cat.getId();

  // get the seeding list once, we need it later...
  CatMngr cm{db};
  PlayerPairList seeding = cm.getSeeding(cat);

  bool hasModifications = true;
  while (hasModifications)   // repeat until we find no more changes to make
  {
    hasModifications = false;

    //
    // Check 1: fill-in names for initial matches (from seeding list)
    //
    QString where;
    where = "%1=%2 AND %3 IS NULL AND %4>0 AND %5>0 AND %6 IS NULL AND %7 IS NULL";
    where = where.arg(BV_CatRef);
    where = where.arg(catId);
    where = where.arg(BV_MatchRef);
    where = where.arg(BV_InitialRank1, BV_InitialRank2);
    where = where.arg(BV_Pair1Ref, BV_Pair2Ref);
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
    where = where.arg(BV_CatRef);  // %1
    where = where.arg(catId);       // %2
    where = where.arg(BV_MatchRef);  // %3
    where = where.arg(BV_InitialRank1, BV_InitialRank2);  // %4, %5
    where = where.arg(BV_Pair1Ref, BV_Pair2Ref);   // %6, %7

    for (BracketVisElement el : getObjectsByWhereClause<BracketVisElement>(where.toUtf8().constData()))
    {
      // Pair 1:
      // is there any match pointing to this bracket element
      // as winner or loser?
      auto parentElem = getParentPlayerPairForElement(el, 1);
      if(parentElem)
      {
        el.linkToPlayerPair(*parentElem, 1);
        hasModifications = true;
      }

      // Pair 2:
      // is there any match pointing to this bracket element
      // as winner or loser?
      parentElem = getParentPlayerPairForElement(el, 2);
      if(parentElem)
      {
        el.linkToPlayerPair(*parentElem, 2);
        hasModifications = true;
      }
    }
  }
}

//----------------------------------------------------------------------------

void BracketVisData::clearExplicitPlayerPairReferences(const PlayerPair& pp) const
{
  // This is the opposite of fillMissingPlayerNames():
  //
  // All explicit references to a specific player pair are erased from the
  // bracket visualization data. This affects only elements without match
  // references ("gap elements").
  //
  // We need this function after changing a match score and the new score
  // results in a change of winner / loser. In this case, the existing
  // explicit references to the affected player pairs are not correct anymore.
  //
  // So we erase the references here and they will be re-filled automatically
  // by the next call to fillMissingPlayerNames() based on the updated match result

  int catId = cat.getId();

  WhereClause w;
  w.addCol(BV_CatRef, catId);
  w.addCol(BV_Pair1Ref, pp.getPairId());
  for (TabRowIterator it{db, TabBracketVis, w}; it.hasData(); ++it)
  {
    it->updateToNull(BV_Pair1Ref);
  }

  w.clear();
  w.addCol(BV_CatRef, catId);
  w.addCol(BV_Pair2Ref, pp.getPairId());
  for (TabRowIterator it{db, TabBracketVis, w}; it.hasData(); ++it)
  {
    it->updateToNull(BV_Pair2Ref);
  }
}

//----------------------------------------------------------------------------

BracketVisData::BracketVisData(const TournamentDB& _db, const Category& _cat)
: TournamentDatabaseObjectManager(_db, TabBracketVis), cat(_cat)
{
}

//----------------------------------------------------------------------------

std::optional<PlayerPair> BracketVisData::getParentPlayerPairForElement(const BracketVisElement& el, int pos) const
{
  // check parameter range
  if ((pos != 1) && (pos != 2)) return {};

  // check element validity
  if (el.getCategoryId() != cat.getId()) return {};

  int elemId = el.getBracketElementId();

  // search for a bracket element that uses this element as next winner / loser match
  Sloppy::estring where = "%1=%2 AND ((%3=%4 AND %5=%6) OR (%7=%4 AND %8=%6))";
  where.arg(BV_CatRef);                    // %1
  where.arg(cat.getId());               // %2
  where.arg(BV_NextWinnerMatch);          // %3
  where.arg(elemId);                      // %4
  where.arg(BV_NextMatchPosForWinner);  // %5
  where.arg(pos);                           // %6
  where.arg(BV_NextLoserMatch);           // %7
  where.arg(BV_NextMatchPosForLoser);   // %8
  auto parentElem = getSingleObjectByWhereClause<BracketVisElement>(where);

  // case 1: no parent
  if (!parentElem) return {};

  // case 2: parent has a match assigned
  auto ma = parentElem->getLinkedMatch();
  if (ma)
  {
    // case 2a: match is not finished, winner and loser are unknown
    if (ma->is_NOT_InState(ObjState::MA_Finished)) return {};

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
  assert(!ma);
  bool hasFixedPair1 = parentElem->getLinkedPlayerPair(1).has_value();
  bool hasFixedPair2 = parentElem->getLinkedPlayerPair(2).has_value();
  if (hasFixedPair1 ^ hasFixedPair2)    // exclusive OR... only one pair is allowed
  {
    if (parentElem->getNextBracketElementForWinner() == elemId)
    {
      assert(parentElem->getNextBracketElementPosForWinner() == pos);
      return (hasFixedPair1) ? parentElem->getLinkedPlayerPair(1) : parentElem->getLinkedPlayerPair(2);
    }
  }

  // default: don't know or can't decide
  return {};
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



std::optional<Match> BracketVisElement::getLinkedMatch() const
{
  auto matchId = row.getInt2(BV_MatchRef);
  if (!matchId) return {};
  MatchMngr mm{db};
  return mm.getMatch(*matchId);
}

//----------------------------------------------------------------------------

Category BracketVisElement::getLinkedCategory() const
{
  int catId = row.getInt(BV_CatRef);
  CatMngr cm{db};
  return cm.getCategoryById(catId);
}

//----------------------------------------------------------------------------

std::optional<PlayerPair> BracketVisElement::getLinkedPlayerPair(int pos) const
{
  if ((pos != 1) && (pos != 2)) return {};

  std::optional<int> pairId;
  if (pos == 1) pairId = row.getInt2(BV_Pair1Ref);
  else pairId = row.getInt2(BV_Pair2Ref);

  if (!pairId) return {};

  return PlayerPair{db, *pairId};
}

//----------------------------------------------------------------------------

bool BracketVisElement::linkToMatch(const Match& ma) const
{
  Category myCat = getLinkedCategory();
  if (ma.getCategory() != myCat) return false;

  row.update(BV_MatchRef, ma.getId());
  return true;
}

//----------------------------------------------------------------------------

bool BracketVisElement::linkToPlayerPair(const PlayerPair& pp, int pos) const
{
  if ((pos != 1) && (pos != 2)) return false;

  Category myCat = getLinkedCategory();
  auto ppCat = pp.getCategory(db);
  if (!ppCat) return false;

  if ((*ppCat) != myCat) return false;

  int pairId = pp.getPairId();
  if (pairId <= 0) return false;

  if (pos == 1)
  {
    row.update(BV_Pair1Ref, pairId);
  } else {
    row.update(BV_Pair2Ref, pairId);
  }

  return true;
}

//----------------------------------------------------------------------------

BracketVisElement::BracketVisElement(const TournamentDB& _db, int rowId)
  :TournamentDatabaseObject(_db, TabBracketVis, rowId)
{

}

//----------------------------------------------------------------------------

BracketVisElement::BracketVisElement(const TournamentDB& _db, const TabRow& _row)
  :TournamentDatabaseObject(_db, _row)
{

}

//----------------------------------------------------------------------------

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

  orientation = (visData[6] == -1) ? BracketOrientation::Left : BracketOrientation::Right;

  switch (visData[7])
  {
  case 1:
    terminator = BracketTerminator::Outwards;
    break;
  case -1:
    terminator = BracketTerminator::Inwards;
    break;
  default:
    terminator = BracketTerminator::None;
  }
  terminatorOffsetY = visData[8];
}

//----------------------------------------------------------------------------

void RawBracketVisDataDef::addPage(BracketPageOrientation orientation, BracketLabelPos labelPos)
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
  return true;
}

//----------------------------------------------------------------------------

std::tuple<BracketPageOrientation, BracketLabelPos> RawBracketVisDataDef::getPageInfo(int idxPage) const
{
  if (idxPage >= getNumPages())
  {
    throw std::range_error("Attempt to access invalid page in bracket visualization data!");
  }

  return std::make_tuple(pageOrientationList.at(idxPage), labelPosList.at(idxPage));
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

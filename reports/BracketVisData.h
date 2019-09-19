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

#ifndef BRACKETVISDATA_H
#define	BRACKETVISDATA_H

#include <memory>

#include <QList>

#include "TournamentDatabaseObjectManager.h"
#include "TournamentDatabaseObject.h"
#include "TournamentDB.h"
#include <SqliteOverlay/TabRow.h>
#include "Category.h"
#include "Match.h"
#include "MatchMngr.h"

namespace QTournament
{
  enum class BracketLabelPos
  {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    None
  };

  enum class BracketOrientation
  {
    Left,
    Right
  };

  enum class BracketTerminator
  {
    None,
    Inwards,
    Outwards
  };

  enum class BracketPageOrientation
  {
    Landscape,
    Portrait
  };

  //----------------------------------------------------------------------------

  class RawBracketVisElement
  {
  public:
    int page;
    int gridX0;
    int gridY0;
    int ySpan;
    int yPageBreakSpan;    // y units until page break; must be 0 < yPageBreakSpan < ySpan
    int nextPageNum;       // if element spans multiple pages; zero-based index
    BracketOrientation orientation;
    BracketTerminator terminator;
    int terminatorOffsetY;

    // we need to repeat the initial-rank-information here,
    // because the information is used differently for the visualization and
    // for the match generation.
    int initialRank1;
    int initialRank2;

    // we need to repeat the original linking information between
    // the bracket elements (bracket matches) here, because later
    // it is impossible to restore it only from the match information
    //
    // the next two values are either...
    //   * > 0 if they refer to the bracketMatchId for the next Match; or
    //   * == 0 if the player drops out without any final rank
    //   * < 0 the if player reaches a final rank (e. g. "-5" for rank 5)
    int nextMatchForWinner;
    int nextMatchForLoser;

    int nextMatchPlayerPosForWinner;  // 1 or 2 for player 1 or player 2
    int nextMatchPlayerPosForLoser;   // 1 or 2 for player 1 or player 2


    RawBracketVisElement(int visData[10]);
    RawBracketVisElement() = default;
    ~RawBracketVisElement() = default;
  };

  //----------------------------------------------------------------------------

  class RawBracketVisDataDef
  {
  public:
    void addPage(BracketPageOrientation orientation, BracketLabelPos labelPos);
    bool addElement(const RawBracketVisElement& el);

    int getNumPages() const { return pageOrientationList.size();}
    int getNumElements() const { return bracketElementList.size();}

    std::tuple<BracketPageOrientation, BracketLabelPos> getPageInfo(int idxPage) const;
    RawBracketVisElement getElement(int idxElement) const;

    void clear();

  private:
    QList<BracketPageOrientation> pageOrientationList;
    QList<BracketLabelPos> labelPosList;
    QList<RawBracketVisElement> bracketElementList;

  };

  //----------------------------------------------------------------------------

  class BracketVisElement : public TournamentDatabaseObject
  {
    friend class BracketVisData;
    friend class TournamentDatabaseObjectManager;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;

  public:
    // getters
    int getPage() const {return row.getInt(BV_Page);}
    int getGridX0() const {return row.getInt(BV_GridX0);}
    int getGridY0() const {return row.getInt(BV_GridY0);}
    int getSpanY() const {return row.getInt(BV_SpanY);}
    int getYPageBreakSpan() const {return row.getInt(BV_YPagebreakSpan);}
    int getNextPageNum() const {return row.getInt(BV_NextPageNum);}

    BracketOrientation getOrientation() const {return static_cast<BracketOrientation>(row.getInt(BV_Orientation));}
    BracketTerminator getTerminator() const {return  static_cast<BracketTerminator>(row.getInt(BV_Terminator));}
    int getTerminatorOffset() const {return row.getInt(BV_Terminator_OFFSET_Y);}

    int getInitialRank1() const {return row.getInt(BV_InitialRank1);}
    int getInitialRank2() const {return row.getInt(BV_InitialRank2);}

    int getNextBracketElementForWinner() const {return row.getInt(BV_NextWinnerMatch);}
    int getNextBracketElementForLoser() const {return row.getInt(BV_NextLoserMatch);}
    int getNextBracketElementPosForWinner() const {return row.getInt(BV_NextMatchPosForWinner);}
    int getNextBracketElementPosForLoser() const {return row.getInt(BV_NextMatchPosForLoser);}

    int getBracketElementId() const {return row.getInt(BV_ElementId);}

    int getCategoryId() const {return row.getInt(BV_CatRef);}

    std::optional<Match> getLinkedMatch() const;
    Category getLinkedCategory() const;
    std::optional<PlayerPair> getLinkedPlayerPair(int pos) const;

    // setters
    bool linkToMatch(const Match& ma) const;
    bool linkToPlayerPair(const PlayerPair& pp, int pos) const;

    BracketVisElement (const QTournament::TournamentDB& _db, int rowId);
    BracketVisElement (const QTournament::TournamentDB& _db, const SqliteOverlay::TabRow& _row);
  };
  using BracketVisElementList = std::vector<BracketVisElement>;

  //----------------------------------------------------------------------------

  class BracketVisData : public TournamentDatabaseObjectManager
  {
  public:

    static std::optional<BracketVisData> getExisting(const Category& _cat);
    static std::optional<BracketVisData> createNew(const Category& _cat, BracketPageOrientation orientation, BracketLabelPos firstPageLabelPos);

    int getNumPages() const;

    std::tuple<BracketPageOrientation, BracketLabelPos> getPageInfo(int idxPage) const;
    BracketVisElementList getVisElements(int idxPage=-1);
    std::optional<BracketVisElement> getVisElement(int idx) const;

    void addPage(BracketPageOrientation pageOrientation, BracketLabelPos labelOnPagePosition) const;
    void addElement(int idx, const RawBracketVisElement& el);

    void fillMissingPlayerNames() const;
    void clearExplicitPlayerPairReferences(const PlayerPair& pp) const;

    BracketVisData(const QTournament::TournamentDB& _db, const Category& _cat);

  private:
    const Category cat;  // DO NOT USE REFERENCES HERE, because this report might out-live the caller and its local objects
    QList<int> labelPosOnPage;

    std::optional<PlayerPair> getParentPlayerPairForElement(const BracketVisElement& el, int pos) const;
  };

}
#endif	/* BRACKETVISDATA_H */


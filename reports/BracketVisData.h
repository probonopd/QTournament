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
  enum class BRACKET_LABEL_POS
  {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    RefereeMode::None
  };

  enum class BRACKET_ORIENTATION
  {
    LEFT,
    RIGHT
  };

  enum class BRACKET_TERMINATOR
  {
    RefereeMode::None,
    INWARDS,
    OUTWARDS
  };

  enum class BRACKET_PAGE_ORIENTATION
  {
    LANDSCAPE,
    PORTRAIT
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
    BRACKET_ORIENTATION orientation;
    BRACKET_TERMINATOR terminator;
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
    void addPage(BRACKET_PAGE_ORIENTATION orientation, BRACKET_LABEL_POS labelPos);
    bool addElement(const RawBracketVisElement& el);

    int getNumPages() const { return pageOrientationList.size();}
    int getNumElements() const { return bracketElementList.size();}

    std::tuple<BRACKET_PAGE_ORIENTATION, BRACKET_LABEL_POS> getPageInfo(int idxPage) const;
    RawBracketVisElement getElement(int idxElement) const;

    void clear();

  private:
    QList<BRACKET_PAGE_ORIENTATION> pageOrientationList;
    QList<BRACKET_LABEL_POS> labelPosList;
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
    int getPage() const {return row.getInt(BV_PAGE);}
    int getGridX0() const {return row.getInt(BV_GRID_X0);}
    int getGridY0() const {return row.getInt(BV_GRID_Y0);}
    int getSpanY() const {return row.getInt(BV_SPAN_Y);}
    int getYPageBreakSpan() const {return row.getInt(BV_Y_PAGEBREAK_SPAN);}
    int getNextPageNum() const {return row.getInt(BV_NEXT_PAGE_NUM);}

    BRACKET_ORIENTATION getOrientation() const {return static_cast<BRACKET_ORIENTATION>(row.getInt(BV_ORIENTATION));}
    BRACKET_TERMINATOR getTerminator() const {return  static_cast<BRACKET_TERMINATOR>(row.getInt(BV_TERMINATOR));}
    int getTerminatorOffset() const {return row.getInt(BV_TERMINATOR_OFFSET_Y);}

    int getInitialRank1() const {return row.getInt(BV_INITIAL_RANK1);}
    int getInitialRank2() const {return row.getInt(BV_INITIAL_RANK2);}

    int getNextBracketElementForWinner() const {return row.getInt(BV_NEXT_WINNER_MATCH);}
    int getNextBracketElementForLoser() const {return row.getInt(BV_NEXT_LOSER_MATCH);}
    int getNextBracketElementPosForWinner() const {return row.getInt(BV_NEXT_MATCH_POS_FOR_WINNER);}
    int getNextBracketElementPosForLoser() const {return row.getInt(BV_NEXT_MATCH_POS_FOR_LOSER);}

    int getBracketElementId() const {return row.getInt(BV_ELEMENT_ID);}

    int getCategoryId() const {return row.getInt(BV_CONFIGREF);}

    std::unique_ptr<Match> getLinkedMatch() const;
    Category getLinkedCategory() const;
    std::unique_ptr<PlayerPair> getLinkedPlayerPair(int pos) const;

    // setters
    bool linkToMatch(const Match& ma) const;
    bool linkToPlayerPair(const PlayerPair& pp, int pos) const;


  private:
    BracketVisElement (const QTournament::TournamentDB& _db, int rowId);
    BracketVisElement (const QTournament::TournamentDB& _db, SqliteOverlay::TabRow row);
  };
  using BracketVisElementList = std::vector<BracketVisElement>;
  using upBracketVisElement = std::unique_ptr<BracketVisElement>;

  //----------------------------------------------------------------------------

  class BracketVisData : public TournamentDatabaseObjectManager
  {
  public:

    static std::unique_ptr<BracketVisData> getExisting(const Category& _cat);
    static std::unique_ptr<BracketVisData> createNew(const Category& _cat, BRACKET_PAGE_ORIENTATION orientation, BRACKET_LABEL_POS firstPageLabelPos);

    int getNumPages() const;

    std::tuple<BRACKET_PAGE_ORIENTATION, BRACKET_LABEL_POS> getPageInfo(int idxPage) const;
    BracketVisElementList getVisElements(int idxPage=-1);
    upBracketVisElement getVisElement(int idx) const;

    void addPage(BRACKET_PAGE_ORIENTATION pageOrientation, BRACKET_LABEL_POS labelOnPagePosition) const;
    void addElement(int idx, const RawBracketVisElement& el);

    void fillMissingPlayerNames() const;
    void clearExplicitPlayerPairReferences(const PlayerPair& pp) const;

  private:
    BracketVisData(const QTournament::TournamentDB& _db, const Category& _cat);
    Category cat;
    QList<int> labelPosOnPage;

    std::unique_ptr<PlayerPair> getParentPlayerPairForElement(const BracketVisElement& el, int pos) const;
  };
  using upBracketVisData = std::unique_ptr<BracketVisData>;


}
#endif	/* BRACKETVISDATA_H */


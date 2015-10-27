/*
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef BRACKETVISDATA_H
#define	BRACKETVISDATA_H

#include <memory>

#include <QList>

#include "GenericObjectManager.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
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
    NONE
  };

  enum class BRACKET_ORIENTATION
  {
    LEFT,
    RIGHT
  };

  enum class BRACKET_TERMINATOR
  {
    NONE,
    INWARDS,
    OUTWARDS
  };

  enum class BRACKET_PAGE_ORIENTATION
  {
    LANDSCAPE,
    PORTRAIT
  };

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

    RawBracketVisElement(int visData[9]);
    RawBracketVisElement() = default;
    ~RawBracketVisElement() = default;
  };

  class RawBracketVisDataDef
  {
  public:
    void addPage(BRACKET_PAGE_ORIENTATION orientation, BRACKET_LABEL_POS labelPos);
    bool addElement(const RawBracketVisElement& el);

    int getNumPages() const { return pageOrientationList.size();}
    int getNumElements() const { return bracketElementList.size();}

    tuple<BRACKET_PAGE_ORIENTATION, BRACKET_LABEL_POS> getPageInfo(int idxPage) const;
    RawBracketVisElement getElement(int idxElement) const;

    void clear();

  private:
    QList<BRACKET_PAGE_ORIENTATION> pageOrientationList;
    QList<BRACKET_LABEL_POS> labelPosList;
    QList<RawBracketVisElement> bracketElementList;

  };

  class BracketVisElement : public GenericDatabaseObject
  {
    friend class BracketVisData;
    friend class GenericObjectManager;

  public:
    int getPage() const {return row[BV_PAGE].toInt();}
    int getGridX0() const {return row[BV_GRID_X0].toInt();}
    int getGridY0() const {return row[BV_GRID_Y0].toInt();}
    int getSpanY() const {return row[BV_SPAN_Y].toInt();}

    BRACKET_ORIENTATION getOrientation() const {return static_cast<BRACKET_ORIENTATION>(row[BV_ORIENTATION].toInt());}
    BRACKET_TERMINATOR getTerminator() const {return  static_cast<BRACKET_TERMINATOR>(row[BV_TERMINATOR].toInt());}

    int getInitialRank1() const {return row[BV_INITIAL_RANK1].toInt();}
    int getInitialRank2() const {return row[BV_INITIAL_RANK2].toInt();}

    unique_ptr<Match> getLinkedMatch() const;

    //bool linkToMatch(const Match& ma);

  private:
    BracketVisElement (TournamentDB* _db, int rowId);
    BracketVisElement (TournamentDB* _db, dbOverlay::TabRow row);
  };
  typedef QList<BracketVisElement> BracketVisElementList;


  class BracketVisData : public GenericObjectManager
  {
  public:

    static unique_ptr<BracketVisData> getExisting(const Category& _cat);
    static unique_ptr<BracketVisData> createNew(const Category& _cat, BRACKET_PAGE_ORIENTATION orientation, BRACKET_LABEL_POS firstPageLabelPos);

    int getNumPages() const;

    tuple<BRACKET_PAGE_ORIENTATION, BRACKET_LABEL_POS> getPageInfo(int idxPage) const;
    BracketVisElementList getVisElements(int idxPage=-1);

    void addPage(BRACKET_PAGE_ORIENTATION pageOrientation, BRACKET_LABEL_POS labelOnPagePosition) const;
    void addElement(const RawBracketVisElement& el);

  private:
    BracketVisData(TournamentDB* _db, const Category& _cat);
    DbTab visTab;
    Category cat;
    QList<int> labelPosOnPage;
  };
  typedef unique_ptr<BracketVisData> upBracketVisData;


}
#endif	/* BRACKETVISDATA_H */


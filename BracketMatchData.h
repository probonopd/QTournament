#ifndef BRACKETMATCHDATA_H
#define BRACKETMATCHDATA_H

#include <variant>

#include "PlayerPair.h"
#include "TournamentDataDefs.h"


namespace QTournament::SvgBracket
{
  // forward
  class SvgBracketDef;

  /** \brief The role of a player pair (in particular in a bracket)
   */
  enum class PairRole
  {
    AsWinner,
    AsLoser,
    Initial
  };

  /** \brief Information on where a player came from in a bracket
   */
  struct IncomingBracketLink
  {
    BracketMatchNumber srcMatch;
    PairRole role;
  };

  /** \brief Information about an outgoing link for a player
   * pair in a bracket
   */
  struct OutgoingBracketLink
  {
    BracketMatchNumber dstMatch;  ///< the bracket match number of the next match
    int pos;   ///< "1": the player becomes player 1 of the next match, "2": guess...
  };

  /** \brief Internal, intermediate struct that captures all important aspects
   * of a bracket match
   */
  class BracketMatchData
  {
  public:
    enum class BranchState
    {
      Dead,
      Alive,
      Assigned
    };

    /** \brief Default ctor for an (invalid) bracket match data struct */
    BracketMatchData() = default;

    /** \brief Standard ctor for a bracket match with initial ranks; the round
     * number is implicitly set to "1".
     */
    BracketMatchData(
        const BracketMatchNumber& n,   ///< the bracket-internal, 1-based number of this match
        const std::string& canoName, ///< the canonical name for this match
        const Rank& p1InitialRank,   ///< the initial rank of player pair 1
        const Rank& p2InitialRank,   ///< the initial rank of player pair 2
        const std::variant<OutgoingBracketLink, Rank>& dstWinner,   ///< what happens to the winner
        const std::variant<OutgoingBracketLink, Rank>& dstLoser   ///< what happens to the loser
        );

    /** \brief Standard ctor for a bracket match in a round >= 2
     */
    BracketMatchData(
        const BracketMatchNumber& n,   ///< the bracket-internal, 1-based number of this match
        const Round& r,   ///< the bracket-internal, 1-based round number for this match
        const std::string& canoName, ///< the canonical name for this match
        const IncomingBracketLink& inLink1,   ///< where player 1 comes from
        const IncomingBracketLink& inLink2,   ///< where player 2 comes from
        const std::variant<OutgoingBracketLink, Rank>& dstWinner,   ///< what happens to the winner
        const std::variant<OutgoingBracketLink, Rank>& dstLoser   ///< what happens to the loser
        );

    /** \brief Default copy ctor */
    BracketMatchData(const BracketMatchData& other) = default;

    /** \brief Default move ctor */
    BracketMatchData(BracketMatchData&& other) = default;

    /** \brief Default copy assignment */
    BracketMatchData& operator=(const BracketMatchData& other) = default;

    /** \brief Default copy assignment */
    BracketMatchData& operator=(BracketMatchData&& other) = default;

    BracketMatchNumber matchNum() const { return brMaNum; }
    Round round() const { return _round; }
    PlayerPairRefId assignedPair1() const { return p1Pair; }
    PlayerPairRefId assignedPair2() const { return p2Pair ;}
    BranchState pair1State() const { return p1State; }
    BranchState pair2State() const { return p2State; }

    const OutgoingBracketLink& nextWinnerMatch() const { return std::get<OutgoingBracketLink>(winnerAction); }
    const OutgoingBracketLink& nextLoserMatch() const { return std::get<OutgoingBracketLink>(loserAction); }
    const IncomingBracketLink& inLink1() const { return std::get<IncomingBracketLink>(src1); }
    const IncomingBracketLink& inLink2() const { return std::get<IncomingBracketLink>(src2); }
    std::optional<Rank> winnerRank() const { return (std::holds_alternative<Rank>(winnerAction) ? std::get<Rank>(winnerAction) : std::optional<Rank>{}); }
    std::optional<Rank> loserRank() const { return (std::holds_alternative<Rank>(loserAction) ? std::get<Rank>(loserAction) : std::optional<Rank>{}); }
    const std::string& canoName() const { return canonicalName; }


    /** \returns an empty optional if the requested player doesn't have
     * an initial rank
     */
    std::optional<Rank> initialRank(
        int pos
        ) const;

    /** \brief Assigns a player pair */
    void assignPlayerPair(
        const PlayerPairRefId& ppId,   ///< the pair ID to assign
        int pos   ///< assign to player "1" or "2"
        );

    /** \brief Declares a player pair as unused
     */
    void setPairUnused(
        int pos   ///< pair "1" or "2"
        );

    /** \returns `true` if both players have no pair assigned
     */
    constexpr bool noPairsAssigned() const { return ((p1Pair.get() == -1) && (p2Pair.get() == -1)); }

    /** \returns `true` if there is a follow-up match for the winner
     */
    constexpr bool hasWinnerMatch() const { return std::holds_alternative<OutgoingBracketLink>(winnerAction); }

    /** \returns `true` if there is a follow-up match for the loser
     */
    constexpr bool hasLoserMatch() const { return std::holds_alternative<OutgoingBracketLink>(loserAction); }

    /** \return `true` if a match can be skipped; that's the case if at least one of the branches is dead.
     */
    constexpr bool canSkip() const { return ((p1State == BranchState::Dead) || (p2State == BranchState::Dead)); }

    /** \returns the object status as a human-friendly string
     */
    std::string dump() const;

    /** \brief dumps the object state to a stream (e.g, cout)
     */
    friend std::ostream& operator<< (std::ostream& out, const BracketMatchData& bmd);

  protected:

  private:
    BracketMatchNumber brMaNum{-1};   ///< the number of the associated match in the bracket
    Round _round{-1};   ///< the 1-based, bracket-internal round this match is played in
    std::string canonicalName;   ///< the unique canonical name of the match like "a2", "b5", "c3", ...

    // static structure information
    std::variant<OutgoingBracketLink, Rank> winnerAction{Rank{-1}};   ///< what happens to the winner
    std::variant<OutgoingBracketLink, Rank> loserAction{Rank{-1}};   ///< what happens to the loser
    std::variant<IncomingBracketLink, Rank> src1{Rank{-1}};   ///< where player 1 comes from
    std::variant<IncomingBracketLink, Rank> src2{Rank{-1}};   ///< where player 2 comes from

    // dynamic allocation information
    BranchState p1State{BranchState::Alive};
    BranchState p2State{BranchState::Alive};
    PlayerPairRefId p1Pair{-1};
    PlayerPairRefId p2Pair{-1};
  };

  /** \brief A sorted list of bracket matches with additional manipulation functions
   * that ensure that the match data (especially the linking) is kept consistent
   */
  class BracketMatchDataList : public std::vector<BracketMatchData>
  {
  public:
    // inherit all parent ctors
    using std::vector<BracketMatchData>::vector;

    /** \brief Takes a sorted list of player pairs (seed) and
     *   * assignes PlayerPair IDs to the first matches as far as possible; and
     *   * tags unused matches
     *
     * \pre The seeding list is sorted with the strongest player at index zero.
     *
     * \pre The seeding list contains at least two player pairs and not more than twice the
     * number of matches in the first round.
     *
     */
    void applySeeding(
        const QTournament::PlayerPairList& seed   ///< the sorted list of initial player pairs
        );

    /** \brief Takes a list of matches and
     *   * assignes PlayerPair IDs to the applicable bracket matches as far as possible; and
     *   * fills gaps between matches (e.g., forwar players over skip, un-played matches)
     *
     * \pre The match list may only contain matches of the applicable category
     *
     * \pre The caller MUST call `applySeeding()` first to get all the fast-forwards right!
     *
     */
    void applyMatches(const std::vector<Match>& maList   ///< the list of matches to apply to the bracket
        );

    /** \brief Follows the bracket in forward direction (==> towards higher rounds)
     * until it finds the next playable match (==> both branches non-dead) starting
     * from a given match as a winner or loser.
     *
     * \returns a BracketMatchNumber if such a match could be found or the resulting final rank
     */
    std::variant<OutgoingBracketLink, Rank> traverseForward(
        const BracketMatchData& ma,   ///< the starting point of the traversal
        PairRole role   ///< start the traversal as a winner or loser
        ) const;

    /** \returns a non-const reference to the bracket match with a given number
     */
    reference findByMatchNumber(
        const BracketMatchNumber& brNum   ///< the bracket match number to search for; MUST BE VALID
        );

    /** \returns a const reference to the bracket match with a given number
     */
    const_reference findByMatchNumber(
        const BracketMatchNumber& brNum   ///< the bracket match number to search for; MUST BE VALID
        ) const;

  protected:
    /** \brief Takes a match and, based on its pair assignments, propagates
     * winners and losers to their next matches and declares unused branches dead
     */
    void propagateWinnerLoser(BracketMatchData& ma
        , int winnerPos);

    /** \brief Takes the player in the given position and promotes it as to the winner
     * match and declares the loser match as "dead".
     *
     * \pre The given position has an assigned pair.
     *
     * \pre The other position is dead.
     */
    void fastForward(
        BracketMatchData& ma,   ///< the match from which to start
        int pos   ///< the position (1 or 2) that contains the to-be-forwarded pair
        );

    /** \brief Sets the loser branch of a given match to "dead"
     *
     * \pre The given match actually has an outgoing link for the loser
     */
    void declareLoserBranchDead(
        const BracketMatchData& ma   ///< the match of which the loser's branch will be declared "dead"
        );

    /** \brief Takes the player in the given position and propagates it backwards
     * along the winner path in the tree until we find a match find assigned
     * pairs; whenever we "back-assign" the pair to a previous match, the loser
     * branch is declared dead.
     *
     * \pre The given position has an assigned pair.
     */
    /*void propagateBackwardsAlongWinnerPath(const BracketMatchData& ma,   ///< the match from which to start
        int pos   ///< the position (1 or 2) that contains the to-be-back-propagated pair
        );*/
  };

  //----------------------------------------------------------------------------

  /** \brief Converts the tag-based SvgBracketDef into the more
   * application-level based representation BracketMatchData
   *
   * \return all bracket matches, sorted by match number (and thus by round)
   */
  BracketMatchDataList convertToBracketMatches(const SvgBracketDef& def);

}



#endif

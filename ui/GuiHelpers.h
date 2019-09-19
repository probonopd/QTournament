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

#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QString>
#include <QtGlobal>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QSizeF>

namespace QTournament
{
  class PlayerProfile;
  class Player;
  class PlayerPair;
  class Match;
  class Court;
  class MatchScore;
}

namespace GuiHelpers
{
  QString groupNumToString(int grpNum);
  QString groupNumToLongString(int grpNum);
  QString prepCall(const QTournament::Match& ma, const QTournament::Court& co, int nCall=0);

  void drawFormattedText(QPainter* painter, QRect r, const QString& s, int alignmentFlags=Qt::AlignVCenter|Qt::AlignLeft,
                                bool isBold=false, bool isItalics=false, QFont fnt=QFont(), QColor fntColor = QColor(0,0,0),
                                double fntSizeFac = 1.0);
  void drawFormattedText(QPainter* painter, int x0, int yBaseline, const QString& s, bool isBold=false, bool isItalics=false,
                                QFont fnt=QFont(), QColor fntColor = QColor(0,0,0), double fntSizeFac = 1.0);
  void drawFormattedText(QPainter* painter, QRect r, const QString& s, int alignmentFlags=Qt::AlignVCenter|Qt::AlignLeft,
                                bool isBold=false, bool isItalics=false, double fntSizeFac = 1.0);
  QSizeF getFormattedTextSize(QPainter* painter, const QString& s, bool isBold=false, bool isItalics=false,
                                    QFont fnt=QFont(), double fntSizeFac = 1.0);
  QSizeF drawTwoLinePlayerPairNames(QPainter* painter, int topLeftX, int topLeftY, const QTournament::Match& ma, const QString& localWinnerName, const QString localLoserName,
                                         double percLineSpace=1.15, bool isBold=false, bool isItalics=false, QFont fnt=QFont(), QColor fntColor = QColor(0,0,0),
                                         double fntSizeFac = 1.0);
  void drawTwoLinePlayerPairNames_Centered(QPainter* painter, const QRectF rect, const QTournament::Match& ma, const QString& localWinnerName, const QString localLoserName,
                                         double percLineSpace=1.15, bool isBold=false, bool isItalics=false, QFont fnt=QFont(), QColor fntColor = QColor(0,0,0),
                                         double fntSizeFac = 1.0, QColor winnerNameColor = QColor(Qt::green), QColor loserNameColor = QColor(Qt::red));

  QString getStatusSummaryForPlayer(const QTournament::Player& p);
  QString getStatusSummaryForPlayer(const QTournament::Player& p, const QTournament::PlayerProfile& pp);
  QString qdt2durationString(const QDateTime& qdt);
  QString qdt2string(const QDateTime& qdt);

  //----------------------------------------------------------------------------

  /** \brief Looks up a (textual) resource and returns its locale-dependent value.
   *
   * Locale dependend means: if our current locale is "de" we try to retrieve a
   * resource with an appended "_de" before the resource name extension (e.g., before
   * the ".html". If we can't find such a resource or if the locale is not "de", we
   * simply return the base resource as provided by the caller.
   *
   * \returns the content of the "localized" resource or an empty string if we
   * couldn't find any matching resource.
   *
   */
  QString getLocaleDependedResource(const QString& resName);

  //----------------------------------------------------------------------------

  /** \brief Asks the user to confirm a given result for a given match.
   *
   * This function is only for displaying the dialog box. There is absolutely
   * no business logic (consistency checks, ...) inside this function.
   *
   * \return `true` if the user confirmed the result; `false` otherwise
   */
  bool showAndConfirmMatchResult(QWidget* parent, const QTournament::Match& ma, const std::optional<QTournament::MatchScore>& matchResult);

  //----------------------------------------------------------------------------

  /** \brief Looks for pattern like "<option_OptionName>....</option_OptionName>" and either
   * disables the option (by removing complete pattern) or enables the option (be removing
   * the tags only and NOT the text between the tags).
   *
   * The provided string is modified in place.
   *
   * Pattern searching in "non-greedy", so you can safely use the same option tag
   * in multiple places in the input string.
   *
   * \note When using nested options, enable/disable from the "outer" to the "inner" options!
   *
   * \returns `true` if the string has been modified
   */
  bool enableTextOption(
      QString& src,   ///< the string to be modifed
      const QString& optName,   ///< the option's name without the leading "option_"
      bool enable = true  ///< set to `true` to enable the option or to `false` for removing the optional text entirely
      );

  //----------------------------------------------------------------------------

  /** \brief Looks for pattern like "<option_OptionName>....</option_OptionName>" and
   * disables the option by removing complete pattern.
   *
   * The provided string is modified in place.
   *
   * Pattern searching in "non-greedy", so you can safely use the same option tag
   * in multiple places in the input string.
   *
   * \returns `true` if the string has been modified
   */
  bool disableTextOption(
      QString& src,   ///< the string to be modifed
      const QString& optName  ///< the option's name without the leading "option_"
      );

  //----------------------------------------------------------------------------

  /** \brief Looks for ALL pattern like "<option_*>....</option_*>" and
   * disables the option by removing complete pattern.
   *
   * The provided string is modified in place.
   *
   * This is intended to finally remove all unused options from a string
   * in one go.
   *
   * \warning THIS DOES NOT WORK WITH NESTED OPTIONS!
   *
   * \returns `true` if the string has been modified
   */
  bool disableAllTextOptions(
      QString& src   ///< the string to be modifed
      );

  //----------------------------------------------------------------------------

  /** \brief Looks for ALL pattern like "<option_*>....</option_*>" and
   * enables the option by removing the surrounding tags.
   *
   * The provided string is modified in place.
   *
   * This is intended to finally enable all remaining options from a string
   * in one go.
   *
   * \warning THIS DOES NOT WORK WITH NESTED OPTIONS!
   *
   * \returns `true` if the string has been modified
   */
  bool enableAllTextOptions(
      QString& src,   ///< the string to be modifed
      bool enable = true
      );

  //----------------------------------------------------------------------------

  /** \brief Internal helper function for for the enable / disable text option calls */
  bool enableOrDisableTextOption(
      QString& src,   ///< the string to be modifed
      const QString& pattern,
      bool enable
      );


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------



}

#endif // GUIHELPERS_H

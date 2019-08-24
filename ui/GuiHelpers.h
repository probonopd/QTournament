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

  /** \brief Asks the user to confirm a given result for a given match.
   *
   * This function is only for displaying the dialog box. There is absolutely
   * no business logic (consistency checks, ...) inside this function.
   *
   * \return `true` if the user confirmed the result; `false` otherwise
   */
  bool showAndConfirmMatchResult(QWidget* parent, const QTournament::Match& ma, const std::optional<QTournament::MatchScore>& matchResult);
}

#endif // GUIHELPERS_H

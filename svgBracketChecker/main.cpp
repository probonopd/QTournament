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

#include "../SvgBracket.h"
#include "SvgRuleChecks.h"
#include "../TournamentDataDefs.h"

#include <iostream>
#include <fstream>


int main(int argc, char *argv[])
{
  auto is = std::ifstream("/home/volker/src/Qt/QTournament/resources/brackets/SingleElim16.svg");

  std::string str;
  is.seekg(0, std::ios::end);
  str.reserve(is.tellg());
  is.seekg(0, std::ios::beg);
  str.assign((std::istreambuf_iterator<char>(is)),
              std::istreambuf_iterator<char>());

  auto allTags = QTournament::SvgBracket::parseContent(str);

  auto error = QTournament::SvgBracket::consistencyCheck(allTags);
  if (error)
  {
    std::cerr << *error << std::endl;
    return 1;
  }

  auto br = QTournament::SvgBracket::findSvgBracket(QTournament::SvgBracketMatchSys::SingleElim, 10);
  std::cout << br.has_value() << std::endl;

  return 0;
}

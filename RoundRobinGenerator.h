/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#ifndef ROUNDROBINGENERATOR_H
#define ROUNDROBINGENERATOR_H

#include <vector>
#include <tuple>


using namespace std;

namespace QTournament
{

class RoundRobinGenerator
{
public:
  RoundRobinGenerator();
  vector<tuple<int, int>> operator() (int numPlayers, int round);


private:
  int n(int r, int p);
};

}
#endif // ROUNDROBINGENERATOR_H

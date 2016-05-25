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

#ifndef HELPERFUNC_H
#define	HELPERFUNC_H

#include <string>
#include <functional>
#include <vector>

#include <QString>

using namespace std;

namespace QTournament
{
  inline string QString2StdString(const QString& s) {
#ifdef __IS_WINDOWS_BUILD
    return s.toLocal8Bit().constData();
#else
    return s.toUtf8().constData();
#endif
  }

  inline QString stdString2QString(const string& s) {
#ifdef __IS_WINDOWS_BUILD
    return QString::fromLocal8Bit(s.c_str());
#else
    return QString::fromUtf8(s.c_str());
#endif
  }

  template<class T>
  int eraseAllValuesFromVector(vector<T>& vec, const T& val)
  {
    int oldSize = vec.size();
    vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end());
    int newSize = vec.size();
    return oldSize - newSize;
  }

  template<class T>
  void lazyAndInefficientVectorSortFunc(vector<T>& vec, std::function<bool (const T&, const T&)> compFunc)
  {
    int i = 0;
    while (i < (vec.size() - 1))
    {
      bool isRightOrder = compFunc(vec[i], vec[i+1]);
      if (!isRightOrder)
      {
        T tmp = vec[i];   // copy!!
        vec[i] = vec[i+1];
        vec[i+1] = tmp;

        // start all over
        i = 0;
        continue;
      }
      ++i;
    }
  }

}
#endif	/* HELPERFUNC_H */


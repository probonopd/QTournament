/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#ifndef CUSTOMMETATYPES_H
#define CUSTOMMETATYPES_H

#include <QObject>

#include "../TournamentDataDefs.h"

/*
 * Registration of custom enums to be used with signals and slots
 */
Q_DECLARE_METATYPE(QTournament::ObjState)

/** \brief Dynamically registers my custom types at runtime; has to
 * be called once before my custom types are first used in signals or slots
 */
void registerCustomTypes();


#endif

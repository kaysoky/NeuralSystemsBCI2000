////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: Global BCI2000 macros and constants.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef DEFINES_H
#define DEFINES_H

#define KEY_BCI2000             "SOFTWARE\\BCI2000"
#define KEY_OPERATOR            "OPERATOR"
#define KEY_VISUALIZATION       "VISUALIZATION"
#define KEY_PARAMETERS          "PARAMETERS"
#define KEY_CONFIG              "CONFIG"
#define KEY_VIEWER              "VIEWER"
#define KEY_EXPORT              "EXPORT"

#ifdef TODO
# error Move remaining SourceID constants into VisID class.
#endif // TODO

namespace SourceID
{
  enum SourceID
  {
    min = 52,

    Classifier = 57,
    Normalizer = 58,
    Statistics = 59,

    EMALG = 83,
    EMALGLOG = 84,

    ExtendedFormat = 255
  };
};

#ifdef TODO
# error Replace these by including <cstdint> when switching to C++11.
#endif // TODO

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
#if( defined( __BORLANDC__ ) && ( __BORLANDC__ <= 0x0560 ) ) // bcc32 <= 5.5.1
typedef unsigned __int64 uint64_t;
typedef signed __int64 int64_t;
#else // __BORLANDC__
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
#endif // __BORLANDC__

typedef float float32_t;

#if 1
typedef uint8_t uint8;
typedef int8_t sint8;
typedef uint16_t uint16;
typedef int16_t sint16;
typedef uint32_t uint32;
typedef int32_t sint32;
typedef uint64_t uint64;
typedef int64_t sint64;
typedef float32_t float32;
#endif

#endif // DEFINES_H

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

#if _MSC_VER // temporary solution until switch to C++11
# include "../extlib/fieldtrip/buffer/src/win32/stdint.h"
#else
# include <stdint.h>
#endif

typedef float float32_t;

// Backward compatibility (for new code, use stdint types):
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


#define KEY_BCI2000             "SOFTWARE\\BCI2000"
#define KEY_OPERATOR            "OPERATOR"
#define KEY_VISUALIZATION       "VISUALIZATION"
#define KEY_PARAMETERS          "PARAMETERS"
#define KEY_CONFIG              "CONFIG"
#define KEY_VIEWER              "VIEWER"
#define KEY_EXPORT              "EXPORT"

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

#endif // DEFINES_H

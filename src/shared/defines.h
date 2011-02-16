////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: Global BCI2000 macros and constants.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

namespace CfgID
{
  enum CfgID
  {
    None = 251, // -5

    Top = 252, // -4
    Left = 253, // -3
    Width = 254, // -2
    Height = 255, // -1
    WindowTitle = 1,

      // Graph options
      MinValue,
      MaxValue,
      NumSamples,
      Reserved_1, // was XAxisLabel
      Reserved_2, // was YAxisLabel
      ChannelGroupSize,
      GraphType,
        // Graph types
        Polyline,
          // Polyline options
          ShowBaselines,
          ChannelColors,
        Field2d,

      // Units
      SampleUnit,
      ChannelUnit,
      ValueUnit,

      // Memo options
      NumLines,

      // Label lists
      ChannelLabels,
      GroupLabels,
      XAxisLabels,
      YAxisLabels,
      // Marker lists
      XAxisMarkers,
      YAxisMarkers,
      // Miscellaneous
      ShowSampleUnit,
      ShowChannelUnit,
      ShowValueUnit,
      SampleOffset,

      Visible,
      InvertedDisplay,
  };
};

namespace SourceID
{
  enum SourceID
  {
    Classifier = 57,
    Normalizer = 58,
    Statistics = 59,

    EMALG = 83,
    EMALGLOG = 84,

    ExtendedFormat = 255
  };
};

typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;
#if( defined( __BORLANDC__ ) && ( __BORLANDC__ <= 0x0560 ) ) // bcc32 <= 5.5.1
typedef unsigned __int64 uint64;
typedef signed __int64 sint64;
#else // __BORLANDC__
typedef unsigned long long uint64;
typedef signed long long sint64;
#endif // __BORLANDC__
typedef float float32;

#endif // DEFINES_H

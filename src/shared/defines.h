////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: Global BCI2000 macros and constants.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DEFINES_H
#define DEFINES_H

#define KEY_BCI2000             "\\SOFTWARE\\BCI2000"
#define KEY_OPERATOR            "\\OPERATOR"
#define KEY_VISUALIZATION       "\\VISUALIZATION"
#define KEY_PARAMETERS          "\\PARAMETERS"
#define KEY_CONFIG              "\\CONFIG"
#define KEY_VIEWER              "\\VIEWER"
#define KEY_EXPORT              "\\EXPORT"

namespace CfgID
{
  enum CfgID
  {
    Top = -4,
    Left = -3,
    Width = -2,
    Height = -1,
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
typedef unsigned long long uint64;
typedef signed long long sint64;
typedef float float32;

#endif // DEFINES_H

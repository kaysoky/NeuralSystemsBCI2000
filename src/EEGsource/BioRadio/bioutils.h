////////////////////////////////////////////////////////////////////////////////
// $Id$
// $Log$
// Revision 1.2  2006/07/05 15:21:19  mellinger
// Formatting and naming changes.
//
// Revision 1.1  2006/07/04 18:44:25  mellinger
// Put files into CVS.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BIOUTILS_H
#define BIOUTILS_H

namespace bioutils
{
              // Get port name from index value
  const char* GetPort(int num);
              // Gets the proper value associated with a specifed voltage range
  double      GetBioRadioRangeValue(int range);
              //Gets the index value associated with a specifed voltage range
  int         VRange2IndexOfRange(double vRange);
              // Writes config file for the bioradio
  int         WriteBioRadioConfig(int fs, int bitRes, double vRange, const char *pathFile);
              // Gets internal gain
  double      GetInternalGain(double vRange);
}

#endif // BIOUTILS_H


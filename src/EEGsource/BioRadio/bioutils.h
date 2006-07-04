////////////////////////////////////////////////////////////////////////////////
// $Id$
// $Log$
// Revision 1.1  2006/07/04 18:44:25  mellinger
// Put files into CVS.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BIOUTILS_H
#define BIOUTILS_H

namespace bioutils
{
  // Get port name from index value
  char*  getPort(int num);
  // Gets the proper value associated with a specifed voltage range
  double getBioRadioRangeValue(int range);
  //Gets the index value associated with a specifed voltage range
  int    vRange2IndexOfRange(double vRange);
  // Writes config file for the bioradio
  int    writeBioRadioConfig(int fs, int bitRes, double vRange, const char *pathFile);
  // Gets internal gain
  double getInternalGain(double vRange);
}

#endif // BIOUTILS_H


////////////////////////////////////////////////////////////////////////////////
//
// File: TransmissionFilter.h
//
// Date: Nov 13, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that returns a subset of input channels in its output
//              signal.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TransmissionFilterH
#define TransmissionFilterH

#include "UGenericFilter.h"

#include <vector>

class TransmissionFilter: public GenericFilter
{
 public:
          TransmissionFilter();
  virtual void Preflight( const SignalProperties& InputProperties,
                                SignalProperties& OutputProperties ) const;
  virtual void Initialize2( const SignalProperties& InputProperties,
                            const SignalProperties& OutputProperties );
  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output );
 private:
   typedef std::vector<size_t> mChannelList_type;
   mChannelList_type           mChannelList;
};

#endif // TransmissionFilterH

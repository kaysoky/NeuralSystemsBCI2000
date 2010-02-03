////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Output into a GDF data file.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GDF_OUTPUT_FORMAT_H
#define GDF_OUTPUT_FORMAT_H

#include "EDFOutputBase.h"
#include "Expression/Expression.h"

#include <vector>

class GDFOutputFormat: public EDFOutputBase
{
 public:
          GDFOutputFormat();
  virtual ~GDFOutputFormat();

  virtual void Publish() const;
  virtual void Preflight( const SignalProperties&, const StateVector& ) const;
  virtual void Initialize( const SignalProperties&, const StateVector& );
  virtual void StartRun( std::ostream&, const std::string& );
  virtual void StopRun( std::ostream& );
  virtual void Write( std::ostream&, const GenericSignal&, const StateVector& );

  virtual const char* DataFileExtension() const { return ".gdf"; }

 private:
  struct EventInfo
  {
    long long      SamplePosition;
    unsigned short Code;
  };
  std::vector<EventInfo>      mEvents;
  std::vector<Expression>     mEventConditions;
  std::vector<unsigned short> mEventCodes;
  std::vector<bool>           mPreviousConditionValues;
};

#endif // GDF_OUTPUT_FORMAT_H

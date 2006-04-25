////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: GDFFileWriter.h
//
// Date: Feb 3, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that stores data into a GDF data file.
//
// $Log$
// Revision 1.2  2006/04/25 18:05:21  mellinger
// Some changes for gcc compatibility.
//
// Revision 1.1  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GDFFileWriterH
#define GDFFileWriterH

#include "EDFFileWriterBase.h"
#include "Expression/Expression.h"

#include <vector>

class GDFFileWriter: public EDFFileWriterBase
{
 public:
          GDFFileWriter();
  virtual ~GDFFileWriter();
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const STATEVECTOR&   Statevector );

 private:
  virtual const char* DataFileExtension() const { return ".gdf"; }

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

#endif // GDFFileWriterH

////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that stores data into a GDF data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GDF_FILE_WRITER_H
#define GDF_FILE_WRITER_H

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
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );

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

#endif // GDF_FILE_WRITER_H

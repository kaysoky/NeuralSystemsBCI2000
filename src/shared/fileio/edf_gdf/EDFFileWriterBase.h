////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for EDF/GDF type file writers.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EDF_FILE_WRITER_BASE_H
#define EDF_FILE_WRITER_BASE_H

#include "FileWriterBase.h"

class EDFFileWriterBase: public FileWriterBase
{
 protected: // No instantiation outside derived classes.
          EDFFileWriterBase();
 public:
  virtual ~EDFFileWriterBase();
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );

 protected:
  struct ChannelInfo
  {
    std::string Label,
                TransducerType,
                PhysicalDimension,
                Filtering;
    float       PhysicalMinimum,
                PhysicalMaximum,
                DigitalMinimum,
                DigitalMaximum;
    long        SamplesPerRecord;
    int         DataType;
  };
  typedef std::vector<ChannelInfo> ChannelContainer;
  ChannelContainer& Channels() { return mChannels; }
  long long NumRecords() const { return mNumRecords; }

 private:
  virtual const char* DataFileExtension() const = 0;

  template<typename T> void PutBlock( const GenericSignal&, const StateVector& );

  ChannelContainer         mChannels;
  long long                mNumRecords;
  std::vector<std::string> mStateNames;
};

#endif // EDF_FILE_WRITER_BASE_H

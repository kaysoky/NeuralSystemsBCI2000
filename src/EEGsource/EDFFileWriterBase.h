////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: EDFFileWriterBase.h
//
// Date: Feb 3, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A base class for EDF/GDF type file writers.
//
// $Log$
// Revision 1.2  2006/04/25 18:05:21  mellinger
// Some changes for gcc compatibility.
//
// Revision 1.1  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EDFFileWriterBaseH
#define EDFFileWriterBaseH

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
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const STATEVECTOR&   Statevector );

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

  template<typename T> void PutBlock( const GenericSignal&, const STATEVECTOR& );

  ChannelContainer         mChannels;
  long long                mNumRecords;
  std::vector<std::string> mStateNames;
};

#endif // EDFFileWriterBaseH

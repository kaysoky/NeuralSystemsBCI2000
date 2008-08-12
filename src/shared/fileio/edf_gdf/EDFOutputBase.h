////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for EDF/GDF type output formats.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EDF_OUTPUT_BASE_H
#define EDF_OUTPUT_BASE_H

#include "GenericOutputFormat.h"

class EDFOutputBase: public GenericOutputFormat
{
 protected: // No instantiation outside derived classes.
          EDFOutputBase();
 public:
  virtual ~EDFOutputBase();

  virtual void Publish() const;
  virtual void Preflight( const SignalProperties&, const StateVector& ) const;
  virtual void Initialize( const SignalProperties&, const StateVector& );
  virtual void StartRun( std::ostream&, const std::string& );
  virtual void StopRun( std::ostream& );
  virtual void Write( std::ostream&, const GenericSignal&, const StateVector& );

  virtual const char* DataFileExtension() const = 0;

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
                DigitalMaximum,
                LowPass,
                HighPass,
                Notch,
                ElectrodePosition[3],
                PhysicalDimensionCode;
    long        SamplesPerRecord;
    int         ElectrodeImpedance,
                DataType;
  };
  typedef std::vector<ChannelInfo> ChannelContainer;
  ChannelContainer& Channels() { return mChannels; }
  long long NumRecords() const { return mNumRecords; }

 private:
  template<typename T> void PutBlock( std::ostream&,
                                      const GenericSignal&,
                                      const StateVector& );

  ChannelContainer         mChannels;
  long long                mNumRecords;
  std::vector<std::string> mStateNames;
};

#endif // EDF_OUTPUT_BASE_H

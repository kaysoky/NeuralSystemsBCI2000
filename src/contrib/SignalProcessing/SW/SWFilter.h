////////////////////////////////////////////////////////////////////////////////
//
// File: SWFilter.h
//
// Description: Slow Wave Class Definition
//              written by Dr. Thilo Hinterberger 2000-2001
//              Copyright University of Tuebingen, Germany
//
// Changes:     2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//              Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//              minor reformulations, reformatting.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SWFilterH
#define SWFilterH

#include "GenericFilter.h"
#include "GenericVisualization.h"
#include <vector>

class TSWFilter : public GenericFilter
{
  public:
                 TSWFilter();
    virtual      ~TSWFilter() {}

    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize( const SignalProperties&, const SignalProperties& );
    virtual void Process( const GenericSignal& InputSignal, GenericSignal& OutputSignal );

  private:
    // SW calculation functions
    void AvgToBuffer( const GenericSignal& InputSignal ); // first average: adds to AvgBlockBuffer
    void CorrectTc();
    void AvgToSW( GenericSignal& OutputSignal );          // second average: adds to SWValue
    void CheckArtefacts( const GenericSignal& OutputSignal );
    void NewTrial();

    int                  mLastItiState;

    // SW variables
    int                  mBufferOffset;    // trial starts at this buffer position
    int                  mPosInBuffer;     // actual position related to bufferzero
    unsigned int         mBlocksInTrial;   // only necessary for AvgBufferSize
    unsigned int         mBlockSize;       // internal BlockSize
    unsigned int         mAvgSpan;
    short                mSWCh;            // number of SW channels
    std::vector<int>     mSWInChList,      // contains list of incoming data channels
                         mSWOutChList;     // contains list of outging data channels
    // buffer and data variables
    int                  mAvgBufferSize;
    GenericSignal        mAvgBlockBuffer;  // AvgBlockBuffer[short SWCh][int AvgBufferSize]
    // Tc-correction variables
    float                mTcFactor;
    std::vector<double>  mTcAk; // TcAk[SWCh+ArteCh]
    // artefact variables
    std::vector<float>   mThresholdAmp,    // ThresholdAmp[short SWCh ]
                         mMinValue,        // MinValue[short SWCh]
                         mMaxValue;        // MaxValue[short SWCh]
};

#endif // SWFilterH

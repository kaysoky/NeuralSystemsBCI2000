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

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

class TSetBaseline : public GenericFilter
{
  public:
                   TSetBaseline();
    virtual        ~TSetBaseline() {}

    virtual void   Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void   Initialize();
    virtual void   Process( const GenericSignal* InputSignal, GenericSignal* );

  private:
    int                  mVisualize,
                         mLastBaselineState;
    GenericVisualization mVis;

    // BL variables.
    std::vector<float>   mBLSamples;
    std::vector<bool>    mBaseChList;
    GenericSignal        mBLSignal;
};

class TFBArteCorrection : public GenericFilter
{
  public:
                 TFBArteCorrection();
    virtual      ~TFBArteCorrection() {}
    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize();
    virtual void Process( const GenericSignal* InputSignal, GenericSignal* );

  private:
    int                  mVisualize;
    GenericVisualization mVis;
    
    enum ArteMode
    {
      off = 0,
      linearSubtraction,
      subtractionIfSupporting,
      subtractionWithAbort,
    }                    mArteMode;
    std::vector<int>     mArteChList;     // list of input channels assigned with the corresponding ArteCh
    std::vector<float>   mArteFactorList; // correction factor for channel n
};

class TSW : public GenericFilter
{
  public:
                 TSW();
    virtual      ~TSW() {}
    
    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize();
    virtual void Process( const GenericSignal* InputSignal, GenericSignal* OutputSignal );

  private:
    int                  mVisualize;
    GenericVisualization mVis;

    int mLastItiState;

    // SW variables
    int mBufferOffset; // trial starts at this buffer position
    int mPosInBuffer;  // actual position related to bufferzero
    unsigned int mBlocksInTrial;  // only necessary for AvgBufferSize
    unsigned int mBlockSize;      // internal BlockSize
    unsigned int mAvgSpan;
    short mSWCh;             // number of SW channels
    std::vector<int> mSWInChList,        // contains list of incoming data channels
                     mSWOutChList;      // contains list of outging data channels
    // buffer and data variables
    int mAvgBufferSize;
    GenericSignal mAvgBlockBuffer; // AvgBlockBuffer[short SWCh][int AvgBufferSize]
    // Tc-correction variables
    float mTcFactor;
    std::vector<double> mTcAk; // TcAk[SWCh+ArteCh]
    // artefact variables
    std::vector<float> mThresholdAmp, // ThresholdAmp[short SWCh ]
                       mMinValue,     // MinValue[short SWCh]
                       mMaxValue;     // MaxValue[short SWCh]
    // SW calculation functions
    void AvgToBuffer( const GenericSignal& InputSignal );  // first average: adds to AvgBlockBuffer
    void CorrectTc();
    void AvgToSW( GenericSignal& OutputSignal );      // second average: adds to SWValue
    void CheckArtefacts( const GenericSignal& OutputSignal );
    void NewTrial();
};

#endif // SWFilterH

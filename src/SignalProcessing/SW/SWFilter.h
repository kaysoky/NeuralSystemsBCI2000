//---------------------------------------------------------------------------
#ifndef SWFilterH
#define SWFilterH
#endif
#include "UGenericFilter.h"
#include "UGenericVisualization.h"

//------------------ Slow Wave Class Definition ------------------------
//                  programed by Dr. Thilo Hinterberger 2000
//--------------------------------------------------------------------------------------------------

  class TSetBaseline : public GenericFilter {
  private:
        STATEVECTOR *statevector;
        int BaseBegin, BaseEnd;
    // BL variables
        int BLBlocks;
        int NumChan;
        bool OldBLState;
        bool* BaseChList;
        GenericSignal *BLSignal;     
        int PosInTrial;
        bool Initialized;
        bool visualize;
        GenericVisualization *vis;
  public:
        TSetBaseline(PARAMLIST *paramlist, STATELIST *statelist);
        ~TSetBaseline();
        int Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, GenericSignal *InputSignal, CORECOMM *new_corecomm);
        int Process(GenericSignal *InputSignal);
        GenericSignal* GetBLSignal();
  };

  class TFBArteCorrection : public GenericFilter {
  private:
        STATEVECTOR *statevector;
        bool Initialized;
        short ArteMode;
        short NumChan;
        short *ArteChList; // list of input channels assigned with the corresponding ArteCh
        float *ArteFactorList; // list of input channels assigned with the corresponding correction factor
        bool visualize;
        GenericVisualization *vis;
  public:
        TFBArteCorrection(PARAMLIST *paramlist, STATELIST *statelist);
        ~TFBArteCorrection();
        int Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, CORECOMM *new_corecomm);
        int Process(GenericSignal *InputSignal);
  };

  class TSW : public GenericFilter {
  private:
        bool Initialized;
        STATEVECTOR *statevector;
        int PosInTrial;
    // SW variables
        int BufferOffset; // trial starts at this buffer position
        int PosInBuffer;  // actual position related to bufferzero
        unsigned int SamplingRate;
        unsigned int BlocksInTrial;  // only necessary for AvgBufferSize
        unsigned int BlockSize;      // internal BlockSize
        unsigned int AvgSpan;
        short SWCh;             // number of SW channels
        short* SWInChList;        // contains list of incoming data channels
        short* SWOutChList;      // contains list of outging data channels
    // buffer and data variables
        int AvgBufferSize;
        GenericSignal* AvgBlockBuffer; // AvgBlockBuffer[short SWCh][int AvgBufferSize]
    // Tc-correction variables
        float Tc;
        double* TcAk; // TcAk[SWCh+ArteCh]
        float TcFactor;
    // artefact variables
        float* ThresholdAmp; // ThresholdAmp[short SWCh
        float* MinValue;     // MinValue[short SWCh]
        float* MaxValue;     // MaxValue[short SWCh]
    // SW calculation functions
        void AvgToBuffer(GenericSignal *InputSignal);  // first average: adds to AvgBlockBuffer
        void CorrectTc();
        void AvgToSW(GenericSignal *OutputSignal);      // second average: adds to SWValue
        void CheckArtefacts(GenericSignal *OutputSignal);
        void NewTrial();
        void InitBuffers(unsigned int, unsigned int, short);  // reallocates Buffers
        bool visualize;
        GenericVisualization *vis;

  public:
        TSW(PARAMLIST *paramlist, STATELIST *statelist);
        ~TSW();
        int Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, CORECOMM *new_corecomm);
    // data related functions
        int Process(GenericSignal *InputSignal, GenericSignal *OutputSignal);
        float GetAvgBlockValue(short SWCh, int Position);
        GenericSignal* GetAvgBlockBuffer();
   // parameter related functions
        int GetPosInTrial();
        int GetPosInBuffer();
        int GetAvgBufferSize();
    // buffer variables
        void SetAvgSpan(unsigned int NewAvgSpan) {AvgSpan=NewAvgSpan; }
  };

//---------------------------------------------------------------------------


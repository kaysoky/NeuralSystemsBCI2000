//------------------ Slow Wave Class Definition ------------------------
//                  written by Dr. Thilo Hinterberger 2000-2001
//                  Copyright University of Tuebingen, Germany
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <math.h>
#include "UState.h"
#include "SWFilter.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------TSetBaseline class definitions --------------------------------------
  TSetBaseline::TSetBaseline(PARAMLIST *paramlist, STATELIST *statelist)
  {
    char line[127];

    statelist->AddState2List("Baseline 1 0 0 0\n");

    strcpy(line, "SWFilter float BaseBegin= 1.9 1.9 0 60 // Begin of Baseline in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter float BaseEnd= 2.0 2.0 0 60 // End of Baseline in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter intlist BaseChList= 2 1 1 0 0 1 // 1 to mark that BL is subtracted");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Visualize int VisualizeBaselineFiltering= 1 0 0 1  // visualize baseline filtered signals (0=no 1=yes)");
    paramlist->AddParameter2List( line, strlen(line) );
    Initialized = false;
    // Initialize(paramlist);
  }

  void TSetBaseline::Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, /*GenericSignal *InputSignal,*/ CORECOMM *new_corecomm)
  {
    int AkElements;
    int visualizeyn;
    statevector = Newstatevector;
    corecomm=new_corecomm;

    int BS = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue())/atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    BaseBegin = atof(paramlist->GetParamPtr("BaseBegin")->GetValue())*BS;
    BaseEnd = (atof(paramlist->GetParamPtr("BaseEnd")->GetValue())-0.001)*BS;
    NumChan = paramlist->GetParamPtr("BaseChList")->GetNumValues();
    visualizeyn= atoi(paramlist->GetParamPtr("VisualizeBaselineFiltering")->GetValue() );
    if (Initialized) {
       delete [] BaseChList;
       delete BLSignal;
#ifdef BCI2000_STRICT
       Initialized = false;
#endif // BCI2000_STRICT
    }
   // allocating BL variables
    BaseChList = new bool[NumChan];
    BLSignal = new GenericSignal(NumChan, 1/*InputSignal->MaxElements()*/);
    for (unsigned short n=0; n<NumChan; n++) BLSignal->SetElements(n, 1 /*InputSignal->GetElements(n)*/);
    for (int m=0; m<NumChan; m++) {
        BaseChList[m] = false;
        if (atoi(paramlist->GetParamPtr("BaseChList")->GetValue(m))==1) BaseChList[m] = true;
        AkElements = BLSignal->GetElements(m);
        for (int n=0; n<AkElements; n++) BLSignal->Value[m][n]=0;   // set BLSignal to zero
    }
    if( visualizeyn == 1 )
 {
        visualize=true;
        vis= new GenericVisualization( paramlist, corecomm);
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_WINDOWTITLE, "BaselineFiltered");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MINVALUE, "-100");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MAXVALUE, "100");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_NUMSAMPLES, "256");
 }
 else
 {
        visualize=false;
 }
#ifdef BCI2000_STRICT
    BLBlocks = 0;
#endif // BCI2000_STRICT
    OldBLState = false;
    PosInTrial = -1;
    Initialized = true;
    return;
  }

  TSetBaseline::~TSetBaseline()
  {
    if (Initialized) {
       delete [] BaseChList;
       delete BLSignal;
    }
  }

  GenericSignal* TSetBaseline::GetBLSignal()
  {
   return BLSignal;
  }

  void TSetBaseline::Process(const GenericSignal*, GenericSignal *InputSignal)
  {
   int AkElements;

    ++PosInTrial;
    bool BLState = statevector->GetStateValue("Baseline");
    if (statevector->GetStateValue("BeginOfTrial")==1) PosInTrial = 0;
    if ((PosInTrial>=BaseBegin) && (PosInTrial<=BaseEnd)) statevector->SetStateValue("Baseline",1);
    else statevector->SetStateValue("Baseline",0);

   if (BLState && (!OldBLState)) {   // Begin of the Baselineperiod
        for (short m=0; m<NumChan; m++) {
            AkElements = BLSignal->GetElements(m);
            for (int n=0; n<AkElements; n++) BLSignal->Value[m][n]=0;   // set BLSignal to zero
        }
        BLBlocks = 0;
   }
   int BChannels = NumChan;
   if (InputSignal->Channels()<NumChan) BChannels = InputSignal->Channels();
   if (BLState) {
        for (short m=0; m<BChannels; m++) {
            if (BaseChList[m]) {
                AkElements = BLSignal->GetElements(m);
                for (int n=0; n<AkElements; n++) {
                    BLSignal->Value[m][n] = BLSignal->Value[m][n]*BLBlocks + InputSignal->Value[m][n];
                    BLSignal->Value[m][n] /= BLBlocks+1;
                }
            }
        }
        ++BLBlocks;
   }
   for (short m=0; m<BChannels; m++) {
       if (BaseChList[m]) {
                AkElements = BLSignal->GetElements(m);
                for (int n=0; n<AkElements; n++) {
                InputSignal->Value[m][n] -= BLSignal->Value[m][n];
           }
       }
   }
   OldBLState = BLState;
   if( visualize )
        {
              vis->SetSourceID(81);
              vis->Send2Operator(InputSignal);
        }
  return;
  }


// ----------- TFBArteCorrection class definitions ----------------------------------------

  TFBArteCorrection::TFBArteCorrection(PARAMLIST *paramlist, STATELIST *statelist)
  {
    char line[255];

    statelist->AddState2List("Artifact 1 0 0 0\n");

    strcpy(line, "SWFilter intlist ArteChList= 2 1 -1 2 -1 63 // Assignment of artefact channels, -1: no artifact channel");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter floatlist ArteFactorList= 2 0.15 0 0 -1 1 // Influence of artefact channel on input channel, -1: no artifact channel");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter int ArteMode= 0 1 0 3 // Artefact correction mode, 0 off, 1 continuous, 2 conditioned");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Visualize int VisualizeFBArteCorFiltering= 1 0 0 1  // visualize FBArte corrected signals (0=no 1=yes)");
    paramlist->AddParameter2List( line, strlen(line) );
    Initialized = false;
  }

  TFBArteCorrection::~TFBArteCorrection()
  {
   if (Initialized) {
       delete [] ArteChList;
       delete [] ArteFactorList;
   }
  }

  void TFBArteCorrection::Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, CORECOMM *new_corecomm)
  {
    int visualizeyn;
    statevector = Newstatevector;
    corecomm=new_corecomm;

   if (Initialized) {
       delete [] ArteChList;
       delete [] ArteFactorList;
#ifdef BCI2000_STRICT
       Initialized = false;
#endif // BCI2000_STRICT
   }
   NumChan = paramlist->GetParamPtr("ArteChList")->GetNumValues();
   ArteChList = new short[NumChan];
   for (int m=0; m<NumChan; m++) ArteChList[m] = atoi(paramlist->GetParamPtr("ArteChList")->GetValue(m));
   NumChan = paramlist->GetParamPtr("ArteFactorList")->GetNumValues();
   ArteFactorList = new float[NumChan];
   for (int m=0; m<NumChan; m++) ArteFactorList[m] = atof(paramlist->GetParamPtr("ArteFactorList")->GetValue(m));
   ArteMode = atoi(paramlist->GetParamPtr("ArteMode")->GetValue());
   visualizeyn= atoi(paramlist->GetParamPtr("VisualizeFBArteCorFiltering")->GetValue() );

  if( visualizeyn == 1 )
 {
        visualize=true;
        vis= new GenericVisualization( paramlist, corecomm);
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_WINDOWTITLE, "Artefiltered");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MINVALUE, "-100");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MAXVALUE, "100");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_NUMSAMPLES, "256");
 }
 else
 {
        visualize=false;
 }

   Initialized = true;
   return;
  }

  void TFBArteCorrection::Process(const GenericSignal*, GenericSignal* InputSignal)
  {
   float ControlSignal;
   float ArteSignal;

   for (short m=0; m<NumChan; m++) {
        if ((ArteChList[m]>=0) && (ArteChList[m]<InputSignal->Channels())) {  // valid channel
            for (short n=0; n<InputSignal->GetElements(m); n++) {
                 ControlSignal = InputSignal->Value[m][n];
                 ArteSignal = InputSignal->Value[ArteChList[m]][n]*ArteFactorList[m];
                 if (ArteMode==1) InputSignal->Value[m][n] = ControlSignal-ArteSignal; // linear subtraction
                 if (ArteMode>=2) {
                    if ((ArteSignal*ControlSignal) > 0) {            // if they have same signs
                       if (fabs(ArteSignal) < fabs(ControlSignal)) InputSignal->Value[m][n] = ControlSignal-ArteSignal;  // if Artefact not too big then correct
                       else {                             // artifact is too big
                           InputSignal->Value[m][n] = 0;  // FB is supressed
                           if (ArteMode==3) statevector->SetStateValue("Artifact",1);
                       }
                    }
                 }  // end ArteMode 2
            }
        }  // end valid channel
   }
   if( visualize )
        {
              vis->SetSourceID(82);
              vis->Send2Operator(InputSignal);
        }
   return;
  }

// ----------- TSW class definitions ----------------------------------------
// Initialization functions -------------------------------------------------

  TSW::TSW(PARAMLIST *paramlist, STATELIST *statelist)
  {
    short n;
    char line[255];

    statelist->AddState2List("Artifact 1 0 0 0\n");

// general variables
    strcpy(line, "SWFilter float SWAvgSpan= 0.5 0.5 0 10 // Averaging window in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter intlist SWInChList= 2 0 1 0 0 63 // Channel index of input signal (include artifact channel!)");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter intlist SWOutChList= 2 0 1 0 0 63 // Channel index of output signal (include artifact channel!)");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter floatlist ThresholdAmp= 2 200 800 200 -2000 2000 // Threshold for invalid Trial in uV");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "SWFilter float Tc= 16 16 0 1024 //Time constant filter settings in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Visualize int VisualizeSWFiltering= 1 0 0 1  // visualize SW filtered signals (0=no 1=yes)");
    paramlist->AddParameter2List( line, strlen(line) );
    strcpy(line, "Sequencer float BIPts= 2.0 2.0 0 60 // Baseline-Intervall in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer float FIPts= 5.0 5.0 0 60 // Feedback-Intervall in s");
    paramlist->AddParameter2List(line, strlen(line));

    Initialized = false;
    //Initialize(paramlist);
  }

  TSW::~TSW()
  {
   if (Initialized) {
    delete AvgBlockBuffer;
    delete [] ThresholdAmp;
    delete [] MaxValue;
    delete [] MinValue;
    delete [] SWInChList;
    delete [] SWOutChList;
    delete [] TcAk;
   }
  }

  void TSW::Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, CORECOMM *new_corecomm)
  {
    int n;
    int visualizeyn;
    statevector = Newstatevector;
    corecomm=new_corecomm;

    if (paramlist->GetParamPtr("SamplingRate")) SamplingRate = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
    else SamplingRate = 256;
    int XBlockSize;
    if (paramlist->GetParamPtr("SampleBlockSize"))
        XBlockSize = atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    else XBlockSize = 16;

    int XSWCh = paramlist->GetParamPtr("SWInChList")->GetNumValues();
    if (paramlist->GetParamPtr("SWOutChList")->GetNumValues()!=XSWCh) Application->MessageBox("Number of incoming and outgoing channels are different!", "SW setup error", MB_OK);
    int BS = SamplingRate/XBlockSize;
    int BIPts = atof(paramlist->GetParamPtr("BIPts")->GetValue())*BS;
    int FIPts = atof(paramlist->GetParamPtr("FIPts")->GetValue())*BS;
    AvgSpan = atof(paramlist->GetParamPtr("SWAvgSpan")->GetValue())*BS;

    InitBuffers(XBlockSize, BIPts+FIPts, XSWCh);      // buffer and list initialization

    for (n=0; n<SWCh; n++) {
        SWInChList[n] = atoi(paramlist->GetParamPtr("SWInChList")->GetValue(n));
        SWOutChList[n] = atoi(paramlist->GetParamPtr("SWOutChList")->GetValue(n));
        ThresholdAmp[n] = atof(paramlist->GetParamPtr("ThresholdAmp")->GetValue(n));
    }
// Tc-correction variables
    Tc = atof(paramlist->GetParamPtr("Tc")->GetValue());
    if (Tc==0) TcFactor = 0;
    else TcFactor = 1-exp(double((-int(BlockSize))/(Tc*SamplingRate)));
   visualizeyn= atoi(paramlist->GetParamPtr("VisualizeSWFiltering")->GetValue() );
 if( visualizeyn == 1 )
 {
        visualize=true;
        vis= new GenericVisualization( paramlist, corecomm);
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_WINDOWTITLE, "SWFiltered");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MINVALUE, "-100");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MAXVALUE, "100");
        vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_NUMSAMPLES, "256");
 }
 else
 {
        visualize=false;
 }
    PosInTrial = -1;
   return;
  }


  void TSW::InitBuffers(unsigned int NewBlockSize, unsigned int NewBlocksInTrial, short NewSWCh)
  {
  int n;
  short m;
  bool NewChannels=false;

// deleting old Buffers
  if (Initialized) {
    delete AvgBlockBuffer;
#ifndef BCI2000_STRICT
    if (NewSWCh>SWCh)
#endif // !BCI2000_STRICT
     {
        delete [] SWInChList;
        delete [] SWOutChList;
        delete [] MaxValue;
        delete [] MinValue;
        delete [] TcAk;
        NewChannels=true;
    }
#ifdef BCI2000_STRICT
    Initialized = false;
#endif // BCI2000_STRICT
  }

// Setting new variables
    BlockSize = NewBlockSize;
    BlocksInTrial = NewBlocksInTrial;
    BufferOffset = BlocksInTrial;
    AvgBufferSize = BufferOffset + BlocksInTrial+1;
    SWCh = NewSWCh;

// allocating Buffers
    AvgBlockBuffer=new GenericSignal(SWCh, AvgBufferSize);
    for (n=0; n<AvgBufferSize; n++) for (m=0; m<(SWCh); m++) AvgBlockBuffer->Value[m][n]=0;
    if ((NewChannels) || (!Initialized)) {
   // allocating channel lists
        SWInChList = new short[SWCh];
        SWOutChList = new short[SWCh];
   // artefact variables
        ThresholdAmp = new float[SWCh];
        for (m=0; m<(SWCh); m++) ThresholdAmp[m]=0;
        MaxValue = new float[SWCh];
        for (m=0; m<(SWCh); m++) MaxValue[m]=-1e10;
        MinValue = new float[SWCh];
        for (m=0; m<(SWCh); m++) MinValue[m]=1e10;
   // allocating Tc variables
        TcAk = new double[SWCh];
        for (m=0; m<(SWCh); m++) TcAk[m]=0;
    }
    PosInBuffer = BufferOffset-1;
    Initialized = true;
  }

// Setup functions ------------------------------------------------------------

  int TSW::GetPosInTrial() { return PosInTrial; }

  int TSW::GetPosInBuffer() { return PosInBuffer; }

  int TSW::GetAvgBufferSize() { return AvgBufferSize; }

  float TSW::GetAvgBlockValue(short SWChannel, int Position)
  {
     float Value = 0;
     if ((Position<BufferOffset) && (SWChannel<SWCh)) {
        if (Position<0) Value = AvgBlockBuffer->Value[SWChannel][PosInBuffer];
        else Value = AvgBlockBuffer->Value[SWChannel][Position+BufferOffset];
     }
     return Value;
  }

  GenericSignal* TSW::GetAvgBlockBuffer()
  {
   return AvgBlockBuffer;
  }

// SW calculation----------arithmetic functions---------------------------------------

  void TSW::NewTrial()
  {
       for (int n=0; n<PosInBuffer-BufferOffset; n++) {
           for (short m=0; m<(SWCh); m++) {
                AvgBlockBuffer->Value[m][n+2*BufferOffset-PosInBuffer] = AvgBlockBuffer->Value[m][n+BufferOffset];
           }
       }
       for (short m=0; m<(SWCh); m++) TcAk[m]=0;
       for (short m=0; m<(SWCh); m++) MaxValue[m]=-1e10;
       for (short m=0; m<(SWCh); m++) MinValue[m]=1e10;
       statevector->SetStateValue("Artifact",0);
       PosInTrial = 0;
       PosInBuffer = BufferOffset;
  }

  void TSW::AvgToBuffer(const GenericSignal *InputSignal)
  {
       for (short m=0; m <SWCh ; m ++) {
           float zsum=0;
           for (unsigned int n=0; n < BlockSize; n ++) zsum=zsum+InputSignal->Value[SWInChList[m]][n];
           AvgBlockBuffer->Value[m][PosInBuffer]=zsum / BlockSize;
       }
  }

  void TSW::CorrectTc()
  {
     double aux;

     for (short m=0; m<(SWCh); m++) {
         aux=(TcFactor*(AvgBlockBuffer->Value[m][PosInBuffer]
         +AvgBlockBuffer->Value[m][PosInBuffer-1]-TcAk[m]))/2;
         TcAk[m]=TcAk[m]+aux;
         AvgBlockBuffer->Value[m][PosInBuffer]=AvgBlockBuffer->Value[m][PosInBuffer]+float(TcAk[m]);
     }
  }

  void TSW::AvgToSW(GenericSignal *OutputSignal)
  {
     for (short m=0; m <(SWCh); m ++) {
     float zsum = 0;
        for (unsigned int n=0; n < AvgSpan; n++) {
            zsum = zsum+AvgBlockBuffer->Value[m][PosInBuffer-n];
        }
        OutputSignal->Value[SWOutChList[m]][0] = zsum / AvgSpan;
     }
  }

  void TSW::CheckArtefacts(GenericSignal *OutputSignal)
  {
     for(short m=0; m <(SWCh); m ++) {
         if (ThresholdAmp[m]>0) {
            if (OutputSignal->Value[SWOutChList[m]][0]<MinValue[m]) MinValue[m] = OutputSignal->Value[SWOutChList[m]][0];
            if (OutputSignal->Value[SWOutChList[m]][0]>MaxValue[m]) MaxValue[m] = OutputSignal->Value[SWOutChList[m]][0];
            if ((MaxValue[m]-MinValue[m])>ThresholdAmp[m])
               statevector->SetStateValue("Artifact",1);
         }
     }
  }

  void TSW::Process(const GenericSignal *InputSignal, GenericSignal *OutputSignal)
  {
       ++PosInTrial;
       if (PosInBuffer<(AvgBufferSize-1)) ++PosInBuffer;
       if (statevector->GetStateValue("BeginOfTrial")==1) NewTrial();
       AvgToBuffer(InputSignal);
       CorrectTc();
       AvgToSW(OutputSignal);
       CheckArtefacts(OutputSignal);
       if( visualize )
        {
              vis->SetSourceID(80);
              vis->Send2Operator(OutputSignal);
        }
   return;
  }

//---------------------------------------------------------------------------
//#include "PCHIncludes.h"
#pragma hdrstop

#include "TDTBCI.h"
#include "BCIError.h"

#include "PrecisionTime.h"
using namespace std;

RegisterFilter( TDTBCI, 1);

// Class constructor for TDTBCI
TDTBCI::TDTBCI()
: RPcoX1( NULL ),
RPcoX2(NULL),
  ZBus(NULL),
  dataA(NULL),
  dataB(NULL),
  dataC(NULL),
  dataD(NULL),
  dataE(NULL)
{

    mSourceCh = 0;
    mSampleBlockSize = 0;
    mSamplingRate = 0;
    nChannels = 0;
    nProcessors = 0;
    use2RX5 = false;
    mOffset = 0;
    LPFfreq = 0;
    HPFfreq = 0;
    notchBW = 0;
    TDTsampleRate = 24414.0625;
    TDTgain = 1;
    blockSize = 0;
    TDTbufSize = 32768;
    curindex = 0;
    stopIndex = 0;
    indexMult = 1;
    devAddr[0] = 0;
    devAddr[1] = 0;
    ECGchannel = -1;
    ECGgain = 0;
	
    BEGIN_PARAMETER_DEFINITIONS
        "Source:TDT string CircuitPath= c:\\bci2000\\src\\EEGsource\\TuckerDavis\\chAcquire64.rco 0 0 1024"
		    "//RCO circuit path (inputfile)",
        "Source:TDT float LPFfreq= 256 256 0 1024 "
		    "//Low Pass Filter Frequency",
        "Source:TDT float HPFfreq= 3 3 0 256"
		    "//High Pass Filter Frequency",
        "Source:TDT float notchBW= 10 10 1 30 "
		    "//60 Hz notch filter BW",
        "Source:TDT float TDTgain= 1 1 1 100000000"
		    "//TDT pre-gain",
        "Source:TDT int nProcessors= 5 5 0 5 "
		    "// Number of processors (set the RCO file accordingly!): ",
        "Source:TDT int NumEEGchannels= 1 64 1 64 "
            "// Number of EEG channels to be acquired",
        "Source:TDT intlist FrontPanelList= 0 0 1 16 "
            "// list of front panel components to acquire",
        "Source:TDT float DigitalGain= 1 1 0 % "
            "// the gain in converting from TTL to float",
        "Source:TDT float FrontPanelGain= 1 1 0 % "
            "// the gain in converting from front panel to float",
    END_PARAMETER_DEFINITIONS

    try
    {
		RPcoX1 = new TRPcoX( ( TComponent* )NULL );
		RPcoX2 = new TRPcoX( ( TComponent* )NULL );
        ZBus = new TZBUSx( ( TComponent* )NULL );
    }
    catch( const EOleSysError& instantiationError )
    {
		bcierr << "Could not instantiate TDT ActiveX control ("
			<< instantiationError.Message.c_str() << ")"
			<< endl;
    }
}

// Class destructor
TDTBCI::~TDTBCI()
{
    // ...because memory leaks are bad!

    reset();
	
    Halt();

    if (RPcoX1 != NULL)
        delete RPcoX1;
    if (RPcoX2 != NULL)
        delete RPcoX2;
    if (ZBus!= NULL)
        delete ZBus;

    #ifdef DEBUGLOG
    fclose(logFile);
    #endif
}

void TDTBCI::reset()
{
    if (dataA != NULL)
        delete [] dataA;
    if (dataB != NULL)
        delete [] dataB;
    if (dataC != NULL)
        delete [] dataC;
    if (dataD != NULL)
        delete [] dataD;
    if (dataE != NULL)
        delete [] dataE;

    dataA = dataB = dataC = dataD = dataE = NULL;
}
//
void TDTBCI::Preflight(const SignalProperties&,	SignalProperties& outputProperties)	const
{
	// checks whether the board	works with the parameters requested, and
	// communicates	the	dimensions of its output signal
    if ((Parameter("nProcessors") != 5) && (Parameter("nProcessors") != 2) && (Parameter("nProcessors") != 8))
    {
        bcierr << "The number of processors must be either 2, 5, or 8"<<endl;
        return;
    }

    int devAddrTemp[2];

    WideString interfaceType("GB");
    // connect to the ZBus
    ZBus->ConnectZBUS(interfaceType.c_bstr());
    devAddrTemp[0] = ZBus->GetDeviceAddr(45,1);
    devAddrTemp[1] = ZBus->GetDeviceAddr(50,1);
    short tConnectType;
    if (devAddrTemp[0] > 0)
        tConnectType = 0;
    else if (devAddrTemp[1] > 0)
        tConnectType = 1;
    else
        bcierr << "There does not seem to be a compatible TDT device on the rack. Quitting."<<endl;

    if ((Parameter("nProcessors") == 2) && tConnectType == 0)
    {
        if (Parameter("NumEEGchannels") > 16)
        {
            bcierr << "An RX5 with two processors may only use up to 16 channels."<<endl;
            return;
        }
    }

    if ((Parameter("nProcessors") == 5) && tConnectType == 0)
    {
        if (Parameter("NumEEGchannels") > 64)
        {
            bcierr << "An RX5 with 5 processors may only use up to 64 channels."<<endl;
            return;
        }
    }

    if ((Parameter("nProcessors") == 2) && tConnectType == 1)
    {
        if (Parameter("NumEEGchannels") > 16)
        {
            bcierr << "An RZ2 with two processors may only use up to 64 channels."<<endl;
            return;
        }
    }

    if ((Parameter("nProcessors") == 8) && tConnectType == 1)
    {
        bcierr << "This option has not been implemented yet! Please contact jawilson@cae.wisc.edu for more info"<<endl;
        return;
    }

    if (Parameter("FrontPanelList")->NumValues() > 0)
    {
        Parameter("FrontPanelList"); // do consistency check
        int mTotalChannelsTmp = Parameter("FrontPanelList")->NumValues() + (int)Parameter("NumEEGchannels");
        if (mTotalChannelsTmp != (int)Parameter("SourceCh"))
        {
            bcierr << "When using FrontPanelList components, NumEEGchannels and the number of FrontPanelList entries must add to SourceCh."<<endl;
            return;
        }
    }

    string circuit = Parameter("CircuitPath");

    long* devNum;
    long* devName;


    bciout <<"Connecting to	TDT..."<<endl;
    switch (tConnectType){
        case 0:
            if (!RPcoX1->ConnectRX5(interfaceType.c_bstr(),	1))
                bcierr << "Error connecting	to the RX5.	Use	the	zBuzMon	to ensure you are connected, and that you are using an RX5 Pentusa." <<endl;
            break;
        case 1:
            if (!RPcoX1->ConnectRZ2(interfaceType.c_bstr(),	1))
                bcierr << "Error connecting	to the RZ2.	Use	the	zBuzMon	to ensure you are connected, and that you are using an RX5 Pentusa." <<endl;
            break;
        case 2:
            bcierr << "Unknown connection (this is a software error!)"<<endl;
    }
		
    bciout <<"Loading RCO/RCX file..."<<endl;
    if (!RPcoX1->LoadCOF(WideString(circuit.c_str())))
    {
        bcierr << "Error loading RCO file. Check the file name and path, and that your Pentusa has 5 processors."<<endl;
        //error
    }
    
	RPcoX1->Halt();
    RPcoX2->Halt();
	
	outputProperties = SignalProperties(Parameter( "SourceCh"	), Parameter( "SampleBlockSize"	), SignalType::float32);
}

void TDTBCI::Initialize(const SignalProperties&, const SignalProperties&)
{
	mSourceCh	= Parameter("SourceCh");
	mSampleBlockSize = Parameter("SampleBlockSize");
	mSamplingRate =	Parameter("SamplingRate");
	LPFfreq	= Parameter("LPFfreq");
	HPFfreq	= Parameter("HPFfreq");
	notchBW	= Parameter("notchBW");
	blockSize =	Parameter("SampleBlockSize");
    TDTgain = Parameter("TDTgain");
    nProcessors = Parameter("nProcessors");
    mEEGchannels = Parameter("NumEEGchannels");
	mDigitalGain = (float)Parameter("DigitalGain");
    mFrontPanelGain = (float)Parameter("FrontPanelGain");

    int nSamplesPerSec = floor(TDTsampleRate / mSamplingRate);
    double nSamplingRate = TDTsampleRate / nSamplesPerSec;
    bciout <<"The actual sampling rate is "<<nSamplingRate <<" Hz"<<endl;
	//mOffset	= 0;
	
	WideString LPFfreqTag =	"LPFfreq";
	WideString HPFfreqTag =	"HPFfreq";
	WideString notchBWTag =	"notchBW";
	WideString blockSizeTag	= "blkSize";
    WideString TDTgainTag = "TDTgain";
    WideString nPerTag = "nPer";
    WideString digGainTag = "DigGain";
    WideString frontPanelGainTag = "FrontPanelGain";
	
	//make sure	we are connected
    WideString interfaceType("GB");
    // connect to the ZBus
    ZBus->ConnectZBUS(interfaceType.c_bstr());
    devAddr[0] = ZBus->GetDeviceAddr(45,1);
    devAddr[1] = ZBus->GetDeviceAddr(50,1);
    if (devAddr[0] > 0)
        connectType = 0;
    else if (devAddr[1] > 0)
        connectType = 1;
    else
        bcierr << "There does not seem to be a compatible TDT device on the rack. Quitting."<<endl;

	// set the number of channels
	// the real	number of channels should be a multiple	of four
    int	valuesToRead = mSampleBlockSize*16;
	
	// set filtering stuff
	if (!RPcoX1->SetTagVal(LPFfreqTag.c_bstr(),	LPFfreq))
	{
		bcierr << "Error setting LPF tag." << endl;
	}
	if (!RPcoX1->SetTagVal(HPFfreqTag.c_bstr(),	HPFfreq))
	{
		bcierr << "Error setting HPF tag." << endl;
	}
	if (!RPcoX1->SetTagVal(notchBWTag.c_bstr(),	notchBW))
	{
		bcierr << "Error setting notch BW tag."	<< endl;
	}
    if (!RPcoX1->SetTagVal(TDTgainTag.c_bstr(),	TDTgain))
	{
		bcierr << "Error setting TDT pre-gain."	<< endl;
	}
	
    if (!RPcoX1->SetTagVal(nPerTag.c_bstr(), nSamplesPerSec))
	{
		bcierr << "Error setting TDT sample rate." << endl;
	}

    if (!RPcoX1->SetTagVal(digGainTag.c_bstr(), mDigitalGain))
	{
		bciout << "Error setting Digital Gain gain." << endl;
	}

    if (!RPcoX1->SetTagVal(frontPanelGainTag.c_bstr(), mFrontPanelGain))
	{
		bciout << "Error setting Front Panel gain." << endl;
	}

    // initialize the data buffers
    reset();
    dataA = new float[valuesToRead];

	if ((nProcessors == 5 && connectType == 0) || (connectType == 1))
    {
        // initialize data buffers
        dataB = new float[valuesToRead];
        dataC = new float[valuesToRead];
        dataD = new float[valuesToRead];
    }

    mUseFrontPanel = Parameter("FrontPanelList")->NumValues() > 0;
    if (mUseFrontPanel)
    {
        mFrontPanelChannels = Parameter("FrontPanelList")->NumValues();
        dataE = new float[valuesToRead];
    }

    // reset the hardware and all conditions     
    mOffset = 0;
    #ifdef DEBUGLOG
    logFile = fopen("TDT_debugLog.txt","w");
    #endif    
	// Start TDT
	RPcoX1->Run();
    ZBus->zBusTrigA(0, 0, 5);
}

void TDTBCI::Halt()
{
	//bciout <<"Halting the TDT..."<<endl;
	RPcoX1->Halt();
	// Halt	the	TDT
}

// This	is the meat	of the class; it reads the data	from the TDT and returns it
void TDTBCI::Process(const GenericSignal&, GenericSignal& outputSignal)
{
	int	valuesToRead = mSampleBlockSize*16;
    int curSample = 0;
    unsigned short curTime;
	
    stopIndex = mOffset + valuesToRead;
	
	short* buffer;
	WideString dataTagA("dataA"), dataTagB("dataB"), dataTagC("dataC"),	dataTagD("dataD"), dataTagE("dataE");
    WideString indexA("indexA"), indexB("indexB"), indexC("indexC"), indexD("indexD"), indexE("indexE");

    curindex = RPcoX1->GetTagVal(indexA.c_bstr());

    #ifdef DEBUGLOG
        fprintf(logFile, "Offset: %d\tSI: %d\tCI: %d\t", mOffset, stopIndex, curindex);
    #endif
    int waitCount=0;
    curTime = PrecisionTime::Now();
    if (stopIndex < TDTbufSize)
    {
		while (curindex < stopIndex)
		{
            //Sleep(1);
			curindex = RPcoX1->GetTagVal(indexA.c_bstr());
		}
    }
    else
    {
        stopIndex = stopIndex % (TDTbufSize);
        // this needs to be updated for the buffer wrap-around in the TDT
        bool done = false;
        while (!done)
        {
            //Sleep(1);
            curindex = RPcoX1->GetTagVal(indexA.c_bstr());
            if  (curindex >= stopIndex && curindex < (TDTbufSize - valuesToRead))
                done = true;
        }
    }
    unsigned short tDiff = PrecisionTime::TimeDiff(curTime, PrecisionTime::Now());

    #ifdef DEBUGLOG
        fprintf(logFile, "CI(end): %d\tT1:%d\t", curindex, tDiff);
    #endif

    // read	in each	data buffer
	if(!RPcoX1->ReadTag(dataTagA.c_bstr(), dataA, mOffset, valuesToRead))
	{
		bcierr << "Error reading data from TDT (A)."<<endl;
	}

	if ((nProcessors == 5 && connectType == 0) || connectType == 1)
    {
		if(!RPcoX1->ReadTag(dataTagB.c_bstr(), dataB, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data from TDT (B)."<<endl;
		}
		
		if(!RPcoX1->ReadTag(dataTagC.c_bstr(), dataC, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data from TDT (C)."<<endl;
		}
		
		if(!RPcoX1->ReadTag(dataTagD.c_bstr(), dataD, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data from TDT (D)."<<endl;
		}
    }

    if (mUseFrontPanel > 0)
    {
        if(!RPcoX1->ReadTag(dataTagE.c_bstr(), dataE, mOffset, valuesToRead))
		{
			bcierr << "Error reading data from TDT front panel."<<endl;
		}
    }

    mOffset = (mOffset + valuesToRead) % (TDTbufSize);
    #ifdef DEBUGLOG
    fprintf(logFile, "EndT: %d\tNewOffset: %d\t", mOffset,PrecisionTime::TimeDiff(curTime, PrecisionTime::Now()));
    #endif
    curTime = PrecisionTime::Now();
    for (int ch =0; ch < mEEGchannels; ch++)
    {
        for (int sample = 0; sample < mSampleBlockSize; sample++)
        {
            curSample = sample*16+ch%(16);
            if (ch < mEEGchannels)
            {
                if (ch < 16)
                    outputSignal(ch, sample) = dataA[curSample];
                else if (ch >= 16 && ch < 32)
                    outputSignal(ch, sample) = dataB[curSample-16];
                else if (ch >= 32 && ch < 48)
                    outputSignal(ch, sample) = dataC[curSample-32];
                else if (ch >= 48 && ch < 64)
                    outputSignal(ch, sample) = dataD[curSample-48];
            }
        }
    }
    for (int fCh = 0; fCh < mFrontPanelChannels; fCh++)
    {
        for (int sample = 0; sample < mSampleBlockSize; sample++)
        {
            curSample = sample*16 + ((int)Parameter("FrontPanelList")(fCh)-1);
            outputSignal(mEEGchannels+fCh, sample) = dataE[curSample];
        }
    }
    #ifdef DEBUGLOG
        fprintf(logFile,"WT: %d\n", PrecisionTime::TimeDiff(curTime, PrecisionTime::Now()));
    #endif
    //bciout << "T: " << (unsigned short)PrecisionTime::TimeDiff(curTime, PrecisionTime::Now()) << "ms"<<endl;
	// END DATA	READ
}
//---------------------------------------------------------------------------

void TDTBCI::dropSamples(GenericSignal& outputSignal)
{
    //just write zeros into the data
    for (int ch =0; ch < mSourceCh; ch++)
    {
        for (int sample = 0; sample < mSampleBlockSize; sample++)
        {
            outputSignal(ch, sample) = 0;
        }
    }
}
#pragma	package(smart_init)


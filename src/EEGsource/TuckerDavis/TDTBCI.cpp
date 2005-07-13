//---------------------------------------------------------------------------
#include "PCHIncludes.h"

#pragma hdrstop

#include "TDTBCI.h"
#include "UBCIError.h"
#include "PCHIncludes.h"
#include "UBCIError.h"


#include <stdio.h>
#include <math.h>

using namespace std;

RegisterFilter( TDTBCI, 1);

// Class constructor for TDTBCI
TDTBCI::TDTBCI()
: RPcoX1( NULL ),
RPcoX2(NULL),
ZBus(NULL)

{
    mSoftwareCh = 0;
    mSampleBlockSize = 0;
    mSamplingRate = 0;
    nChannels = 0;
    nChannels1 = 0;
    nChannels2 = 0;
    nProcessors1 = 0;
    nProcessors2 = 0;
    use2RX5 = false;
    mOffset = 0;
    LPFfreq = 0;
    HPFfreq = 0;
    notchBW = 0;
    TDTsampleRate = 24414.0625;
    TDTgain = 1;
    blockSize = 0;
    TDTbufSize = 32000;
    curindex = 0;
    stopIndex = 0;
    indexMult = 1;
	
    BEGIN_PARAMETER_DEFINITIONS
        "Source int SoftwareCh= 64 64 1 128"
		"// The number of channels acquired",
        "Source int SoftwareChBoard1= 64 64 1 64"
		"// Number of channels on first RX5 (ignored if only one RX5)",
        "Source int SoftwareChBoard2= 64 64 1 64"
		"// Number of channels on 2nd RX5 (ignored if only one RX5)",
        "Source int SampleBlockSize= 16 5 1 128"
		"//number of samples transmitted at a time",
        "Source int SamplingRate=   512 128 1 4000"
		"//sample rate",
        "Source string CircuitPath= C:\\bci2000\\EEGsource\\TDTclient\\ 0 0 1024"
		"//RCO circuit path",
        "Source string CircuitName= chAcquire64.rco 0 0 1024"
		"//RCO Circuit name",
        "Source float LPFfreq= 256 256 0 1024"
		"//Low Pass Filter Frequency",
        "Source float HPFfreq= 3 3 0 256"
		"//High Pass Filter Frequency",
        "Source float notchBW= 10 10 1 30"
		"//60 Hz notch filter BW",
        "Source float TDTgain= 1 1 1 32768"
		"//TDT pre-gain",
        "Source int nProcessorsBoard1= 5 5 0 0"
		"// Number of 1st RX5 processors (set the RCO file accordingly!): ",
        "Source int nProcessorsBoard2= 0 5 0 0"
		"// Number of 2nd RX5 processors (0 if only one board): ",
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
    delete [] dataA;
    delete [] dataB;
    delete [] dataC;
    delete [] dataD;
    delete [] dataA2;
    delete [] dataB2;
    delete [] dataC2;
    delete [] dataD2;
	
    Halt();
    
    delete RPcoX1;
    delete RPcoX2;
}

//
void TDTBCI::Preflight(const SignalProperties&,	SignalProperties& outputProperties)	const
{
    PreflightCondition( Parameter( "TransmitCh" ) <= Parameter( "SoftwareCh" ) );
	// checks whether the board	works with the parameters requested, and
	// communicates	the	dimensions of its output signal
	
	bool twoBoards = false;
	
    // check the the number of processors given is valid
    if ((Parameter("nProcessorsBoard1") != 2) && (Parameter("nProcessorsBoard1") != 5))
    {
        bcierr << "The number of processors must be either 2 or 5."<<endl;
    }
	
    if ((Parameter("nProcessorsBoard2") != 2) && (Parameter("nProcessorsBoard2") != 5) && (Parameter("nProcessorsBoard2") != 0))
    {
        bcierr << "The number of processors for the 2nd system must be either 2 or 5, or 0 if not being used."<<endl;
    }
	
	
    // check if a 2nd system is being used
    if (Parameter("nProcessorsBoard2") > 0)
        twoBoards=true;
    else
        twoBoards=false;

	if (!twoBoards)
    {
        if ((Parameter("SoftwareCh") > 16) && (Parameter("nProcessorsBoard1") == 2))
            bcierr << "The maximum number of channels for a 2 processor board must be 16 or less."<<endl;
    }
    else
    {
        if ((Parameter("SoftwareChBoard1") > 16) && (Parameter("nProcessorsBoard1") == 2))
            bcierr << "The maximum number of channels for a 2 processor board must be 16 or less."<<endl;

        if ((Parameter("SoftwareChBoard2") > 16) && (Parameter("nProcessorsBoard2") == 2))
            bcierr << "The maximum number of channels for a 2 processor board must be 16 or less."<<endl;
    }

    if (twoBoards)
	{
		if (Parameter( "SoftwareChBoard1" )+Parameter( "SoftwareChBoard2" ) != Parameter( "SoftwareCh" ))
			bcierr << "If we have two systems, SoftwareChBoard1+SoftwareChBoard2 has to equal SoftwareCh" << endl;
		
	}	
	
	const char * circuitPath = Parameter("CircuitPath");
	const char * circuitName = Parameter("CircuitName");
	
	string circuit(circuitPath);
	WideString interfaceType("GB");
	
	if(	!circuit.empty() &&	'\\' !=	circuit[circuit.size()-1]  ){
		circuit.append("\\");
	}
	
	circuit.append(circuitName);
	
	// connect to the ZBus
    ZBus->ConnectZBUS(interfaceType.c_bstr());
    
    if (!twoBoards)
    {
		bciout <<"Connecting to	Pentusa..."<<endl;
		if (!RPcoX1->ConnectRX5(interfaceType.c_bstr(),	1))
		{
			bcierr << "Error connecting	to the RX5.	Use	the	zBuzMon	to ensure you are connected, and that you are using an RX5 Pentusa." <<endl;
			// error
		}
		
		bciout <<"Loading RCO file..."<<endl;
		if (!RPcoX1->LoadCOF(WideString(circuit.c_str())))
		{
			bcierr << "Error loading RCO file. Check the file name and path, and that your Pentusa has 5 processors."<<endl;
			//error
		}
		
		
    }
    else
    {
        bciout <<"Connecting to	Pentusa #1..."<<endl;
		if (!RPcoX1->ConnectRX5(interfaceType.c_bstr(),	1))
		{
			bcierr << "Error connecting	to the RX5.	Use	the	zBuzMon	to ensure you are connected, and that you are using an RX5 Pentusa." <<endl;
			// error
		}
		
        bciout <<"Connecting to	Pentusa #2..."<<endl;
		if (!RPcoX2->ConnectRX5(interfaceType.c_bstr(),	1))
		{
			bcierr << "Error connecting	to the 2nd RX5.	Use	the	zBuzMon	to ensure you are connected, and that you are using an RX5 Pentusa." <<endl;
			// error
		}
		
		bciout <<"Loading RCO file #1..."<<endl;
		if (!RPcoX1->LoadCOF(WideString(circuit.c_str())))
		{
			bcierr << "Error loading RCO file. Check the file name and path, and that your Pentusa has 5 processors."<<endl;
			//error
		}
        bciout <<"Loading RCO file #2..."<<endl;
		if (!RPcoX2->LoadCOF(WideString(circuit.c_str())))
		{
			bcierr << "Error loading RCO file. Check the file name and path, and that your Pentusa has 5 processors."<<endl;
			//error
		}
    }
	
	//status = RPcoX1->GetStatus();
	
	RPcoX1->Halt();
    RPcoX2->Halt();
	
	outputProperties = SignalProperties(Parameter( "SoftwareCh"	), Parameter( "SampleBlockSize"	), SignalType::int16);
}

void TDTBCI::Initialize()
{
	mSoftwareCh	= Parameter("SoftwareCh");
	mSampleBlockSize = Parameter("SampleBlockSize");
	mSamplingRate =	Parameter("SamplingRate");
	LPFfreq	= Parameter("LPFfreq");
	HPFfreq	= Parameter("HPFfreq");
	notchBW	= Parameter("notchBW");
	blockSize =	Parameter("SampleBlockSize");
    TDTgain = Parameter("TDTgain");
    nChannels1 = Parameter("SoftwareChBoard1");
    nChannels2 = Parameter("SoftwareChBoard2");
    nProcessors1 = Parameter("nProcessorsBoard1");
    nProcessors2 = Parameter("nProcessorsBoard2");
	
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
	
	//make sure	we are connected
	
	if (Parameter("nProcessorsBoard2") > 0)
        use2RX5 = true;
    else
        use2RX5 = false;
	//...
	
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
		bcierr << "Error setting TDT sample rate."	<< endl;
	}
	
    // set up the second system if necessary
    if (use2RX5)
    {
		// set filtering stuff
		if (!RPcoX2->SetTagVal(LPFfreqTag.c_bstr(),	LPFfreq))
		{
			bcierr << "Error setting LPF tag." << endl;
		}
		if (!RPcoX2->SetTagVal(HPFfreqTag.c_bstr(),	HPFfreq))
		{
			bcierr << "Error setting HPF tag." << endl;
		}
		if (!RPcoX2->SetTagVal(notchBWTag.c_bstr(),	notchBW))
		{
			bcierr << "Error setting notch BW tag."	<< endl;
		}
		if (!RPcoX2->SetTagVal(TDTgainTag.c_bstr(),	TDTgain))
		{
			bcierr << "Error setting TDT pre-gain."	<< endl;
		}
		if (!RPcoX2->SetTagVal(nPerTag.c_bstr(), nSamplesPerSec))
		{
			bcierr << "Error setting TDT sample rate."	<< endl;
		}
    }
	
    // initialize the data buffers
    dataA = new float[valuesToRead];
	
	if (nProcessors1 == 5)
    {
        // initialize data buffers
        dataB = new float[valuesToRead];
        dataC = new float[valuesToRead];
        dataD = new float[valuesToRead];
    }
	
    if (use2RX5)
    {
        dataA2 = new float[valuesToRead];
		
        if (nProcessors2 == 5)
        {
            dataB2 = new float[valuesToRead];
            dataC2 = new float[valuesToRead];
            dataD2 = new float[valuesToRead];
        }
    }
	
    // reset the hardware and all conditions
    ZBus->zBusTrigA(0, 0, 5);
    mOffset = 0;
    
	// Start TDT
	RPcoX1->Run();
}

void TDTBCI::Halt()
{
	bciout <<"Halting the TDT..."<<endl;
	RPcoX1->Halt();
	// Halt	the	TDT
}

// This	is the meat	of the class; it reads the data	from the TDT and returns it
void TDTBCI::Process(const GenericSignal*, GenericSignal* outputSignal)
{
	int	valuesToRead = mSampleBlockSize*16;
    int curSample = 0;
	
    stopIndex = mOffset + valuesToRead;
	
	short* buffer;
	WideString dataTagA("dataA"), dataTagB("dataB"), dataTagC("dataC"),	dataTagD("dataD");
    WideString indexA("indexA"), indexB("indexB"), indexC("indexC"), indexD("indexD");
	
    curindex = RPcoX1->GetTagVal(indexA.c_bstr());
	
    if (stopIndex < TDTbufSize)
    {
		while (curindex < stopIndex)
		{
			curindex = RPcoX1->GetTagVal(indexA.c_bstr());
			Sleep(0);
		}
    }
    else
    {
        stopIndex = stopIndex % TDTbufSize;
        // this needs to be updated for the buffer wrap-around in the TDT
        bool done = false;
        while (!done)
        {
			curindex = RPcoX1->GetTagVal(indexA.c_bstr());
            if  (curindex >= stopIndex && curindex < (TDTbufSize - valuesToRead))
                done = true;
        }
    }
	
    // read	in each	data buffer
	if(!RPcoX1->ReadTag(dataTagA.c_bstr(), dataA, mOffset, valuesToRead))
	{
		bcierr <<	"Error reading data	from Pentusa (A)."<<endl;
	}
	
	if (nProcessors1 == 5)
    {
		if(!RPcoX1->ReadTag(dataTagB.c_bstr(), dataB, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data	from Pentusa (B)."<<endl;
		}
		
		if(!RPcoX1->ReadTag(dataTagC.c_bstr(), dataC, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data	from Pentusa (C)."<<endl;
		}
		
		if(!RPcoX1->ReadTag(dataTagD.c_bstr(), dataD, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data	from Pentusa (D)."<<endl;
		}
    }
	
    if (use2RX5)
    {
		if(!RPcoX2->ReadTag(dataTagA.c_bstr(), dataA2, mOffset, valuesToRead))
		{
			bcierr <<	"Error reading data	from Pentusa2 (A)."<<endl;
		}
		
		if (nProcessors1 == 5)
		{
			if(!RPcoX2->ReadTag(dataTagB.c_bstr(), dataB2, mOffset, valuesToRead))
			{
				bcierr <<	"Error reading data	from Pentusa2 (B)."<<endl;
			}
			
			if(!RPcoX2->ReadTag(dataTagC.c_bstr(), dataC2, mOffset, valuesToRead))
			{
				bcierr <<	"Error reading data	from Pentusa2 (C)."<<endl;
			}
			
			if(!RPcoX2->ReadTag(dataTagD.c_bstr(), dataD2, mOffset, valuesToRead))
			{
				bcierr <<	"Error reading data	from Pentusa2 (D)."<<endl;
			}
		}
    }
	
    // update the index and offset
    mOffset = (mOffset + valuesToRead) % (32000);
	
    //Sleep(10);
    for (int ch =0; ch < mSoftwareCh; ch++)
    {
        for (int sample = 0; sample < mSampleBlockSize; sample++)
        {
            curSample = sample*16+ch%16;
			
            //bciout << "("<<curSample<<","<<ch<<","<<sample<<")"<<endl;
            if (ch < 16)
            {
                //bciout <<"0-15"<<endl;
                outputSignal->SetValue(ch, sample, dataA[curSample]);
            }
            else if (ch >= 16 && ch < 32)
            {
                //bciout <<"16-31"<<endl;
                outputSignal->SetValue(ch%16 + 16, sample, dataB[curSample]);
            }
            else if (ch >= 32 && ch < 48)
            {
                //bciout <<"32-47,"<<ch%16+2*nChannels<<","<<dataC[curSample]<<endl;
                outputSignal->SetValue(ch%16 + 32, sample, dataC[curSample]);
            }
            else if (ch >= 48 && ch < 64)
            {
                //bciout <<"48-63"<<endl;
                outputSignal->SetValue(ch%16 + 48, sample, dataD[curSample]);
            }

            if (nProcessors2 == 0)
                continue;

            if (ch >= 64 && ch < 80)
            {
                //bciout <<"0-15"<<endl;
                outputSignal->SetValue(ch%16 + 64, sample, dataA2[curSample]);
            }
            else if (ch >= 80 && ch < 96)
            {
                //bciout <<"16-31"<<endl;
                outputSignal->SetValue(ch%16 + 80, sample, dataB2[curSample]);
            }
            else if (ch >= 96 && ch < 112)
            {
                //bciout <<"32-47,"<<ch%16+2*nChannels<<","<<dataC[curSample]<<endl;
                outputSignal->SetValue(ch%16 + 96, sample, dataC2[curSample]);
            }
            else if (ch >= 112 && ch < 128)
            {
                //bciout <<"48-63"<<endl;
                outputSignal->SetValue(ch%16 + 112, sample, dataD2[curSample]);
            }
        }
    }
	// END DATA	READ
}
//---------------------------------------------------------------------------

#pragma	package(smart_init)


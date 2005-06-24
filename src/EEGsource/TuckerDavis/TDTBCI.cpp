//---------------------------------------------------------------------------
#include "PCHIncludes.h"

#pragma hdrstop

#include "TDTBCI.h"
#include "UBCIError.h"
#include "PCHIncludes.h"
#include "UBCIError.h"
#include "RPCOXLib_OCX.h"

#include <stdio.h>
#include <math.h>

using namespace std;

RegisterFilter( TDTBCI, 1);

// Class constructor for TDTBCI
TDTBCI::TDTBCI()
: RPcoX1( NULL )
{
    mSoftwareCh = 0;
    mSampleBlockSize = 0;
    mSamplingRate = 0;
    nChannels = 0;
    mOffset = 0;
    LPFfreq = 0;
    HPFfreq = 0;
    notchBW = 0;
    TDTsampleRate = 24414.0625;
    TDTgain = 1;
    blockSize = 0;
    curindex = 0;
    stopIndex = 0;
    indexMult = 1;

    BEGIN_PARAMETER_DEFINITIONS
        "Source int SoftwareCh= 64 64 1 128"
            "//this is the number of digitized channels",
        "Source int SampleBlockSize= 16 5 1 128"
            "//number of samples transmitted at a time",
        "Source int SamplingRate=   128 128 1 4000"
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
            "//TDT pre-gain"
    END_PARAMETER_DEFINITIONS

    try
    {
      RPcoX1 = new TRPcoX( ( TComponent* )NULL );
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

    Halt();
    
    delete RPcoX1;
}

//
void TDTBCI::Preflight(const SignalProperties&,	SignalProperties& outputProperties)	const
{
	// checks whether the board	works with the parameters requested, and
	// communicates	the	dimensions of its output signal

	int	sr = Parameter("SamplingRate");           

	const char * circuitPath = Parameter("CircuitPath");
	const char * circuitName = Parameter("CircuitName");

	string circuit(circuitPath);
	WideString interfaceType("GB");

	if(	!circuit.empty() &&	'\\' !=	circuit[circuit.size()-1]  ){
		circuit.append("\\");
	}

	circuit.append(circuitName);

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

	RPcoX1->Halt();
	//status = RPcoX1->GetStatus();



    int nSamplesPerSec = floor(TDTsampleRate / sr);
    double nSamplingRate = TDTsampleRate / nSamplesPerSec;
    Parameter("SamplingRate") = nSamplingRate;
    bciout <<"The actual sampling rate is "<<nSamplingRate <<" Hz"<<endl;

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

    
	//mOffset	= 0;

	WideString nChanTag	= "nChans";
	WideString LPFfreqTag =	"LPFfreq";
	WideString HPFfreqTag =	"HPFfreq";
	WideString notchBWTag =	"notchBW";
	WideString blockSizeTag	= "blkSize";
    WideString TDTgainTag = "TDTgain";

	//make sure	we are connected

	//...

	// set the number of channels
	// the real	number of channels should be a multiple	of four
    int	valuesToRead = mSampleBlockSize*16;
	nChannels =	ceil(mSoftwareCh / 4);
	blockSize *= nChannels;
	if (!RPcoX1->SetTagVal(nChanTag.c_bstr(), nChannels))
	{
		bcierr << "Error setting channel number	tag." << endl;
	}

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

        // initialize data buffers
    dataA = new float[valuesToRead];
    dataB = new float[valuesToRead];
    dataC = new float[valuesToRead];
    dataD = new float[valuesToRead];
    
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

	//mOffset	= (mOffset + valuesToRead) % (2*valuesToRead);
    stopIndex = mOffset + valuesToRead;

	short* buffer;
	WideString dataTagA("dataA"), dataTagB("dataB"), dataTagC("dataC"),	dataTagD("dataD");
    WideString indexA("indexA"), indexB("indexB"), indexC("indexC"), indexD("indexD");

    //float *dataAll = new float[tempNChs][mSampleBlockSize];


    curindex = RPcoX1->GetTagVal(indexA.c_bstr());
    //if (mOffset >= 31000)
    //   bciout << "mOffset is wrong! ("<<mOffset<<", "<<stopIndex<<", "<<curindex<<")"<<endl;


    if (stopIndex < 32000)
    {
       while (curindex < stopIndex)
       {
          curindex = RPcoX1->GetTagVal(indexA.c_bstr());
          Sleep(0);
       }
    }
    else
    {
        // this needs to be updated for the buffer wrap-around in the TDT
        while (curindex != 0)
        {
          curindex = RPcoX1->GetTagVal(indexA.c_bstr());
          Sleep(0);
        }
    }

    // read	in each	data buffer
	if(!RPcoX1->ReadTag(dataTagA.c_bstr(), dataA, mOffset, valuesToRead))
	{
	  bcierr <<	"Error reading data	from Pentusa (A)."<<endl;
	}

	if(!RPcoX1->ReadTag(dataTagB.c_bstr(), dataB, mOffset, valuesToRead))
	{
	  bcierr <<	"Error reading data	from Pentusa (B)."<<endl;
	}

	if(!RPcoX1->ReadTag(dataTagC.c_bstr(), dataC, mOffset, valuesToRead))
	{
	  bcierr <<	"Error reading data	from Pentusa."<<endl;
	}

	if(!RPcoX1->ReadTag(dataTagD.c_bstr(), dataD, mOffset, valuesToRead))
	{
	  bcierr <<	"Error reading data	from Pentusa."<<endl;
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
        }
    }
	// END DATA	READ
}
//---------------------------------------------------------------------------

#pragma	package(smart_init)

AnsiString TDTBCI::buildTarget(int ch)
{
	// currently this is only for the pentusa
	int	maxChannels	= 16;
	int	devNum = floor(ch /	maxChannels);

	AnsiString tag;

	switch (devNum)
	{
	/*
		case 0:
			tag	= "DataA~" +	IntToStr(realCh);
			break;
		case 1:
			tag	= "DataB~" +	IntToStr(realCh);
			break;
		case 2:
			tag	= "DataC~" +	IntToStr(realCh);
			break;
		case 3:
			tag	= "DataD~" +	IntToStr(realCh);
			break;
		default:
			// error of	some kind...
			break;
			*/
		case 0:
			tag	= "dataA"; // +	   IntToStr(realCh);
			break;
		case 1:
			tag	= "dataB";// +	  IntToStr(realCh);
			break;
		case 2:
			tag	= "dataC";// +	  IntToStr(realCh);
			break;
		case 3:
			tag	= "dataD";// +	  IntToStr(realCh);
			break;
		default:
			// error of	some kind...
			break;
	}

	return tag;
}

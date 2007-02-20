/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------
/*  TDTADC Class definition
	Authors: Adam Wilson & Samuel Inverso
    
    Description: The TDTADC class implements functions required for the TDT to
    	operate, such as connect, start, stop, read data, etc.

    Class Properties:
    	(Necessary Internal Stuff)
        currentIndex[] (float) - the current index into channel n
        nChannels (short int) - number of channels
        bufferSize (float) - the size of the buffer to read

        (RCO Circuit Tag Properties)
    	HPFfreq (float)- high pass filter frequency
        LPFfreq (float)- lowpass filter frequency
        notchBW (float)- bandwidth of 60 Hz notch filter
        blkSize (float)- block size of each channel
        scale~n (float)- scale factor for channel n
        index~n (float)- index in data buffer for channel n
        data~n - pointer to data buffer for channel n

        (TDT device parms)
        sampleFreq (float)- sample frequency of device
        circuit (*Widestring) - circuit path
        devType (*Widestring) - interface type ("GB")
        devNum (long) - device number (1 for RX5)

    Class Functions:
    	(TDT Control)
        connect(*devType, devNum) - connects to pentusa
        loadRCO(circuit, sampleFreq) - loads the RCO file
        run() - start TDT
        halt() - halt TDT
        getStatus() - get status
        bool isConnected()
        bool isCOFloaded()


*/
#ifndef TDTADCH
#define TDTADCH

#include <Classes.hpp>
#include <SyncObjs.hpp>
#include <string>
#include <vector>
#include <exception>
#include <wstring.h>

#include "RPCOXLib_OCX.h"


class TDTADC : public TThread{

public:
    __fastcall TDTADC(bool createSuspended);
    __fastcall ~TDTADC();

    bool isConnected();
    void halt();


protected:

    bool high;		// determines which block of data to retrieve
    unsigned int samplesAcquired;
    float curIndex;

    TEvent *acquire; // if we should be acquiring dignal from the adc
    TEvent *executeSafe; // if set means execute is not currently
                          // in the process of getting signal

	WideString* circuit;

};
//---------------------------------------------------------------------------
#endif



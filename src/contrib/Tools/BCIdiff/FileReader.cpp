/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#include "FileReader.h"

#include "BCI2000FileReader.h"
#include <iostream>

FileReader::FileReader()
: mBCI2000Data(NULL)
{
}

FileReader::~FileReader()
{
        delete mBCI2000Data;
}

bool FileReader::openNewFile(const char* fileName)
{
        if(mBCI2000Data)
                delete(mBCI2000Data);
        mBCI2000Data=new BCI2000FileReader();
        int retVal=mBCI2000Data->Open(fileName, 50000).ErrorState();
        /*
        #define BCI2000ERR_NOERR                0
        #define BCI2000ERR_FILENOTFOUND         1
        #define BCI2000ERR_MALFORMEDHEADER      2
        #define BCI2000ERR_NOBUFMEM             3
        #define BCI2000ERR_CHSINCONSISTENT      4
        */
        if(retVal!=BCI2000FileReader::NoError)
        {
           switch (retVal)
           {
                  case BCI2000FileReader::FileOpenError:
                        std::cerr<<"File not found."<<'\n';
                        break;
                  case BCI2000FileReader::MalformedHeader:
                        std::cerr<<"Not a BCI2000 file."<<'\n';
                        break;
                  default:
                        std::cerr<<"Unknown Error!"<<'\n';
           }
           delete mBCI2000Data;
           mBCI2000Data=NULL;
           return false;
        }
        mFileName=fileName;
        return true;
}

int FileReader::getHeaderLength() const
{
        return mBCI2000Data->HeaderLength();
}

int FileReader::getStateVectorLength() const
{
        return mBCI2000Data->StateVectorLength();
}
int FileReader::getNumChannels() const
{
        return mBCI2000Data->SignalProperties().Channels();
}

int FileReader::getSampleFrequency() const
{
        return mBCI2000Data->SamplingRate();
}

unsigned long FileReader::getNumSamples()
{
        return mBCI2000Data->NumSamples();
}

const ParamList* FileReader::getParamListPtr() const
{
        return mBCI2000Data->Parameters();
}

const StateList* FileReader::getStateListPtr() const
{
        return mBCI2000Data->States();
}

const StateVector* FileReader::getStateVectorPtr() const
{
        return mBCI2000Data->StateVector();
}

GenericSignal::ValueType FileReader::readValue(int channel, unsigned long sample)
{
        return mBCI2000Data->RawValue(channel, sample);
}

void FileReader::readStateVector(unsigned long sample)
{
        mBCI2000Data->ReadStateVector(sample);
}

const char* FileReader::getFileName()
{
        return mFileName;
}



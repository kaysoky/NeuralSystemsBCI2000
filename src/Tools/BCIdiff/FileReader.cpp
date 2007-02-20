/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#include "FileReader.h"

#include "UBCI2000Data.h"
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
        mBCI2000Data=new BCI2000DATA();
        int retVal=mBCI2000Data->Initialize(fileName, 50000);
        /*
        #define BCI2000ERR_NOERR                0
        #define BCI2000ERR_FILENOTFOUND         1
        #define BCI2000ERR_MALFORMEDHEADER      2
        #define BCI2000ERR_NOBUFMEM             3
        #define BCI2000ERR_CHSINCONSISTENT      4
        */
        if(retVal!=BCI2000ERR_NOERR)
        {
           switch (retVal)
           {
                  case BCI2000ERR_FILENOTFOUND:
                        std::cerr<<"File not found."<<'\n';
                        break;
                  case BCI2000ERR_MALFORMEDHEADER:
                        std::cerr<<"Not a BCI2000 file."<<'\n';
                        break;
                  case BCI2000ERR_NOBUFMEM:
                        std::cerr<<"Not enough memory availible."<<'\n';
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
        return mBCI2000Data->GetHeaderLength();
}

int FileReader::getStateVectorLength() const
{
        return mBCI2000Data->GetStateVectorLength();
}
int FileReader::getNumChannels() const
{
        return mBCI2000Data->GetNumChannels();
}

int FileReader::getSampleFrequency() const
{
        return mBCI2000Data->GetSampleFrequency();
}

unsigned long FileReader::getNumSamples()
{
        return mBCI2000Data->GetNumSamples();
}

const PARAMLIST* FileReader::getParamListPtr() const
{
        return mBCI2000Data->GetParamListPtr();
}

const STATELIST* FileReader::getStateListPtr() const
{
        return mBCI2000Data->GetStateListPtr();
}

const STATEVECTOR* FileReader::getStateVectorPtr() const
{
        return mBCI2000Data->GetStateVectorPtr();
}

GenericSignal::value_type FileReader::readValue(int channel, unsigned long sample)
{
        return mBCI2000Data->ReadValue(channel, sample);
}

void FileReader::readStateVector(unsigned long sample)
{
        mBCI2000Data->ReadStateVector(sample);
}

const char* FileReader::getFileName()
{
        return mFileName;
}





/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef FileReaderH
#define FileReaderH

#include "GenericSignal.h"

class FileReader{
public:
        FileReader();
        ~FileReader();

        bool openNewFile(const char* fileName);

        int getHeaderLength() const;
        int getStateVectorLength() const;
        int getNumChannels() const;
        int getSampleFrequency() const;
        unsigned long getNumSamples();
        const class ParamList *getParamListPtr() const;
        const class StateList *getStateListPtr() const;
        const class StateVector *getStateVectorPtr() const;
        GenericSignal::ValueType readValue(int channel, unsigned long sample);
        void readStateVector(unsigned long sample);
        const char* getFileName();

private:
        class BCI2000FileReader* mBCI2000Data;
        const char* mFileName;
};
#endif


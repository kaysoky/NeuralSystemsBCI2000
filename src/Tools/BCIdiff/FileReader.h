#ifndef FileReaderH
#define FileReaderH

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
        const class PARAMLIST *getParamListPtr() const;
        const class STATELIST *getStateListPtr() const;
        const class STATEVECTOR *getStateVectorPtr() const;
        short readValue(int channel, unsigned long sample);
        void readStateVector(unsigned long sample);
        const char* getFileName();

private:
        class BCI2000DATA* mBCI2000Data;
        const char* mFileName;
};
#endif


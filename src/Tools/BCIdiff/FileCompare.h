#ifndef FileCompareH
#define FileCompareH

class FileCompare{
public:
        FileCompare();
        ~FileCompare();

        void setFiles(class FileReader* file1, class FileReader* file2);

        bool compareHeaderLengths();
        bool compareStateVectorLengths();
        bool compareNumChannels();
        bool compareSampleFrequencies();
        bool compareNumSamples();
        bool compareParams();
        bool compareStates();
        bool compareValues();

private:
        class FileReader* mFile1, * mFile2;
};
#endif
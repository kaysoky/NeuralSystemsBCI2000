#ifndef FileCompareH
#define FileCompareH

class FileCompare{
public:
        FileCompare();
        ~FileCompare();

        void setFiles(class FileReader* file1, class FileReader* file2);

        bool headerLengthsDiffer();
        bool numChannelsDiffer();
        bool sampleFrequenciesDiffer();
        bool numSamplesDiffer();
        bool paramsDiffer();
        bool stateListsDiffer();
        bool valuesDiffer(bool compareStates);

private:
        bool stateVectorLengthsDiffer();
        bool currentStatesDiffer();
        class FileReader* mFile1, * mFile2;
};
#endif
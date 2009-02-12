/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef FileCompareH
#define FileCompareH

class FileCompare{
public:
        FileCompare();
        ~FileCompare();

        void setFiles(class FileReader* file1, class FileReader* file2);
        void setQuietOption(bool beQuiet);

        bool headerLengthsDiffer();
        bool numChannelsDiffer();
        bool sampleFrequenciesDiffer();
        bool numSamplesDiffer();
        bool paramsDiffer();
        bool stateListsDiffer();
        bool valuesDiffer(bool omitData, bool omitStates, bool omitTimes);

private:
        bool stateVectorLengthsDiffer();
        bool currentStatesDiffer(bool omitTimes);
        class FileReader* mFile1, * mFile2;
        bool mBeQuiet;
};
#endif

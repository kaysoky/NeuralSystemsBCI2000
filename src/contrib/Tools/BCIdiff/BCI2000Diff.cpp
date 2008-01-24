/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#pragma hdrstop

#include "FileCompare.h"
#include "FileReader.h"
#include <iostream>
//---------------------------------------------------------------------------

using namespace std;

#pragma argsused
int main(int argc, char* argv[])
{

        const char usage[] =
                "Usage: BCIdiff <options> <file1> <file2>\n"
                " options are:\n"
                " --help                \tshow this help\n"
                " --no-parameters       \tdon't compare parameters\n"
                " --no-init-states      \tdon't compare initial state lists and state names\n"
                " --no-states           \tdon't compare state lists and state values\n"
                " --no-data             \tdon't compare EEG data\n"
                " --comp-times          \tcompare source and stimulus times\n"
                " The return value of BCIdiff is 1 if differences were found,"
                " 0 for no differences.";

        short fileNameCount=0;
        bool omitParameters=false;
        bool omitInitStates=false;
        bool omitStates=false;
        bool omitData=false;
        bool omitTimes=true;
        bool beQuiet=false;
        for(int i=1;i<argc;++i)
        {
                if(string(argv[i])=="--help")
                {
                        cout<<usage<<'\n';
                        return 0;
                }
                else if(string(argv[i])=="--no-parameters")
                        omitParameters=true;
                else if(string(argv[i])=="--no-init-states")
                        omitInitStates=true;
                else if(string(argv[i])=="--no-states")
                        omitStates=true;
                else if(string(argv[i])=="--no-data")
                        omitData=true;
                else if(string(argv[i])=="--comp-times")
                        omitTimes=false;
                else if(string(argv[i])=="--quiet")
                        beQuiet=true;
                else if(i>=argc-2&&argv[i][0]!='-'&&argv[i][1]!='-')
                        ++fileNameCount;
                else
                {
                        cerr<<usage<<'\n';
                        return -1;
                }
        }

        if(fileNameCount!=2)
        {
                cerr<<"Please provide two filenames!"<<'\n';
                return -1;
        }

        FileReader* fileR1=new FileReader();
        if(!fileR1->openNewFile(argv[argc-2]))
        {
                cerr<<"File one could not be opened. \n";
                delete fileR1;
                return -1;
        }

        FileReader* fileR2=new FileReader();
        if(!fileR2->openNewFile(argv[argc-1]))
        {
                cerr<<"File two could not be opened. \n";
                delete fileR1;
                delete fileR2;
                return -1;
        }

        FileCompare fileC1;
        fileC1.setFiles(fileR1,fileR2);
        fileC1.setQuietOption(beQuiet);

        bool filesDiffer=false;
        filesDiffer|=fileC1.headerLengthsDiffer();
        filesDiffer|=fileC1.numChannelsDiffer();
        filesDiffer|=fileC1.sampleFrequenciesDiffer();
        filesDiffer|=fileC1.numSamplesDiffer();

        if(!omitParameters)
                filesDiffer|=fileC1.paramsDiffer();

        //bool compareStateVectors=false;
        if(!omitInitStates)
        {
                omitStates==fileC1.stateListsDiffer();
          //      filesDiffer|=!compareStateVectors;
        }

        if(!omitData||!omitStates)
                filesDiffer|=fileC1.valuesDiffer(omitData, omitStates, omitTimes);

        delete fileR1;
        delete fileR2;

        return filesDiffer;
}
//---------------------------------------------------------------------------

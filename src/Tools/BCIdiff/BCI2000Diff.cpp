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
        if(argc!=3)
        {
                std::cerr<<"Please provide two filenames!"<<'\n';
                return -1;
        }

        FileReader* fileR1=new FileReader();
        if(!fileR1->openNewFile(argv[1]))
        {
                std::cerr<<"File one could not be opened. \n";
                delete fileR1;
                return -1;
        }

        FileReader* fileR2=new FileReader();
        if(!fileR2->openNewFile(argv[2]))
        {
                std::cerr<<"File two could not be opened. \n";
                delete fileR1;
                delete fileR2;
                return -1;
        }
        FileCompare fileC1;
        fileC1.setFiles(fileR1,fileR2);

        fileC1.headerLengthsDiffer();
        fileC1.numChannelsDiffer();
        fileC1.sampleFrequenciesDiffer();
        fileC1.numSamplesDiffer();
        fileC1.paramsDiffer();
        bool compareStateVectors=!fileC1.stateListsDiffer();
        bool compareTimes=false;
        fileC1.valuesDiffer(compareStateVectors, compareTimes);

        delete fileR1;
        delete fileR2;

        return 1;
}
//---------------------------------------------------------------------------

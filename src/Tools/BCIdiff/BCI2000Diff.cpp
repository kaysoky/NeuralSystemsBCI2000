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

        fileC1.compareHeaderLengths();
        fileC1.compareStateVectorLengths();
        fileC1.compareNumChannels();
        fileC1.compareSampleFrequencies();
        fileC1.compareNumSamples();
        fileC1.compareParams();
        fileC1.compareStates();
        fileC1.compareValues();

        delete fileR1;
        delete fileR2;

        return 0;
}
//---------------------------------------------------------------------------

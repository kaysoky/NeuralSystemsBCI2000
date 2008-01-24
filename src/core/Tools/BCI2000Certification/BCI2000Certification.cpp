/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused
#include <vector>
#include <string>
#include <stdio.h>

using namespace std;

#include "CertLauncher.h"
#include "Functions.h"

int main(int argc, char* argv[])
{
    CertLauncher CT;

    string datDir, resFile;
    double thresh;
    vector<basicStats> minReqs;
    if (!parseCfg(thresh,resFile, datDir, minReqs))
    {
        exit(-1);
    }

    //get files to delete
    vector<string> fNames;
    parseDir(datDir, fNames);
    string in;
    if (fNames.size() > 0)
    {
        cout <<"Previous test results exist in "<<datDir<<". Do you want to remove them? "<<endl;
        cin >> in;
        if (in[0] == 'y' || in[0] == 'Y')
        {
            for (int i = 0; i < fNames.size(); i++)
            {
                remove(fNames[i].c_str());
            }
        }
        else
            cout <<"These files will be included in the analysis."<<endl;
    }


    if (!CT.parseIni())
    {
        cout <<"Some configurations in the BCI2000Certification.ini file are incomplete.\nThe following valid configurations will be run:"<<endl;
        for (int i=0; i < CT.nTasks(); i++)
            if (!CT[i].skip)
                cout << "* " <<CT[i].taskName<<endl;

        cout<<"------------------------------\nDo you want to continue (Y/N)?"<<endl;
        cin >> in;
        if (in[0] != 'y' && in[0] != 'Y')
            exit(0);
    }

    while (CT.tasksRemain())
    {
        CT.launchProgs();
        CT.monitorProgs();
        CT.nextTask();
    }

    //now decide whether to launch the analysis
    cout <<"\n\n-----------------------"<<endl;
    cout<<"Testing is complete. Do you want to run the analysis program (Y/N)?"<<endl;
    cin >> in;
    if (in[0] != 'y' && in[0] != 'Y')
        exit(0);

    string comm("start BCI2000CertAnalysis.exe");
    system(comm.c_str());
    return 0;
}
//---------------------------------------------------------------------------

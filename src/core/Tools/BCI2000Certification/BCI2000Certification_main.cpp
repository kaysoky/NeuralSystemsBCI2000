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
        cout << "Unable to find BCI2000Certification.cfg, or the file is configured incorrectly. Press enter to exit."<<endl;
        char c[255];
        cin.getline(c,1);
        exit(-1);
    }

    //get files to delete
    vector<string> fNames;
    parseDir(datDir, fNames);
    string in;

    if (!CT.parseIni())
    {
        if (CT.taskReturnCode() == -1)
        {
            cout << "Unable to find BCI2000Certification.ini. Press enter to exit."<<endl;
            char c[255];
            cin.getline(c,1);
            exit(-1);
        }
        else if (CT.taskReturnCode() == -3)
        {
            cout << "Duplicate task names found in BCI2000Certification.ini. Remove or rename duplicates, and try again. Press enter to exit."<<endl;
            char c[255];
            cin.getline(c,1);
            exit(-3);
        }
        cout <<"Some configurations in the BCI2000Certification.ini file are incomplete.\nThe following valid configurations will be run:"<<endl;
        for (int i=0; i < CT.nTasks(); i++)
            if (!CT[i].skip)
                cout << "* " <<CT[i].taskName<<endl;

        cout<<"------------------------------\nDo you want to continue (Y/N)?"<<endl;
        cin >> in;
        if (in[0] != 'y' && in[0] != 'Y')
            exit(0);
    }
    
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




    CT.nextTask();
    while (CT.tasksRemain())
	{
		Sleep(2000);
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

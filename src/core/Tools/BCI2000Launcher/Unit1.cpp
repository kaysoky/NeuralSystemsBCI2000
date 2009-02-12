/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <stdio.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "VCLDefines.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
using namespace std;
TmainForm *mainForm;
//---------------------------------------------------------------------------
__fastcall TmainForm::TmainForm(TComponent* Owner)
    : TForm(Owner)
{
    sourceList->PopupMenu = sourcePopup;
    sigProcList->PopupMenu = spPopup;
    appList->PopupMenu = appPopup;
    othersList->PopupMenu = othersPopup;


    char curdirTmp[200];
    current_directory(curdirTmp);
    progDir = curdirTmp;
    curdir = curdirTmp;
    curdir.append("\\");
    iniFile = progDir + "\\BCI2000Launcher.ini";
    helpLoc = curdir.c_str();
    helpLoc.Insert("BCIlauncher_help.chm", curdir.length()+1);
}
//---------------------------------------------------------------------------




void __fastcall TmainForm::FormCreate(TObject */*Sender*/)
{
    //lists of programs
    bool haveFile = false;
    int boxDlg;
    vector<string> lines;
    string label, value, tmp;
    ifstream in;
    in.open(iniFile.c_str());

    if (!in.fail())
        haveFile = true;
    if (in.fail())
    {
        in.close();
        in.clear();
		boxDlg = Application->MessageBox(
		  VCLSTR( "The BCI2000Launcher.ini file cannot be found, so a new one"
				  " will be created in the BCI2000/prog folder. Press Ignore,"
				  " and move the programs from the Others list to the"
				  " appropriate list (a new ini file will be created),"
				  " or press Retry to select a file." ),
		  VCLSTR( "*.ini file not found" ),
		  MB_ABORTRETRYIGNORE);

        switch (boxDlg)
        {
            case IDABORT:
                exit(0);
                break;
            case IDRETRY:
                //get a new *.ini file
                OpenParmDlg->FileName = iniFile.c_str();
                OpenParmDlg->Filter = "INI File (*.ini)|*.ini";
                if (OpenParmDlg->Execute())
                {
                    in.open(AnsiString(OpenParmDlg->FileName).c_str());
                    if (!in.fail())
                        haveFile = true;
                    else
                    {
                        haveFile = false;
                    }
                }
                break;
            case IDIGNORE:
                //do nothing
                break;
        }
    }

    Show();
    if (haveFile)
    {
        in.seekg(0);
        // load in the BCIlaunch.ini file
        while (getNextLine(in, lines, " \t"))
        {
            label.clear();
            value.clear();
            if (lines.size() != 2)
                continue;
                
            label = lowerCase(lines[0]);
            value = lines[1];

            if (label == "source")
            {
                sourceStr.push_back(lines[1]);
            }
            else if (label == "sigproc")
            {
                SPStr.push_back(lines[1]);
            }
            else if (label == "app")
            {
                appStr.push_back(lines[1]);
            }
            else
            {
				Application->MessageBox(
				  VCLSTR( "The BCI2000launcher.ini file is corrupted, or an"
						  " invalid file was selected." ),
				  VCLSTR( "Invalid INI file" ),
				  MB_OK);
            }
            lines.clear();
        }
        in.close();
    }

    //start by getting the directory listing
    struct ffblk ffblk;
    unsigned int done, count;
    vector<string>::iterator dirPos;
    
    done = findfirst("*.exe", &ffblk,0);
    while(!done)
    {
        dirListing.push_back(ffblk.ff_name);
        //temporarily add the directory name to the others list
        done = findnext(&ffblk);
    }

    //now add the files to the appropriate list boxes
    // do the source programs first
    for (unsigned int i = 0; i < sourceStr.size(); i++)
    {
        if (ismember(sourceStr[i], dirListing))
            sourceList->Items->Add(sourceStr[i].c_str());
    }

    for (unsigned int i = 0; i < SPStr.size(); i++)
    {
        if (ismember(SPStr[i], dirListing))
            sigProcList->Items->Add(SPStr[i].c_str());
    }

    for (unsigned int i = 0; i < appStr.size(); i++)
    {
        if (ismember(appStr[i], dirListing))
            appList->Items->Add(appStr[i].c_str());
    }

    count = 0;
    while (count < dirListing.size())
    {
        if ((!ismember(dirListing[count], appStr)) && (!ismember(dirListing[count], SPStr)) && (!ismember(dirListing[count], sourceStr)))
        {
            if (!(dirListing[count] == "operat.exe") && !(dirListing[count] == "BCI2000launcher.exe"))
            {
                othersList->Items->Add(dirListing[count].c_str());
            }
        }     
        count++;
    }

    //finally, make the first objects highlighted
    if (sourceList->Items->Count > 0)
        sourceList->Selected[0] = true;
    if (sigProcList->Items->Count > 0)
        sigProcList->Selected[0] = true;
    if (appList->Items->Count > 0)
        appList->Selected[0] = true;
}
//---------------------------------------------------------------------------

bool TmainForm::ismember(string str, vector<string> strs)
{
    string tmpStr;
    for (unsigned int i=0; i < strs.size(); i++)
    {
        //if (strcmp(str.c_str(), strs[i].c_str()) == 0)
        if (str == strs[i])
            return true;
    }
    return false;
}



//----------------------------------------------------------------
void __fastcall TmainForm::launchButClick(TObject */*Sender*/)
{
    SetCurrentDir(curdir.c_str());
    statusList->Clear();
    statusList->Items->Add(curdir.c_str());
	statusList->Items->Add("Launching Operator...");

	string fs = "\\";
	stringstream comm;
	comm << "\"" << curdir << "operat.exe\"";

	/*FileRun1->FileName = curdir.c_str();
	FileRun1->FileName.Insert("operat.exe", curdir.length()+1);
    string comm = "";*/
    
	if (parmList->Items->Count > 0 )
	{

		comm << " --OnConnect \"-";

		for (size_t i = 0; i < parmFiles.size(); i++)
		{
			comm << "LOAD PARAMETERFILE ";
			comm << EncodedString(parmFiles[i]) << "; ";
		}
		comm << " SETCONFIG;\"";
	}
	//comm << " ";

	//FileRun1->Parameters = comm.c_str();

	/*if (parmBox->Text.Length() > 0)
    {
        string comm = "--OnConnect \"-LOAD PARAMETERFILE ";
        ostringstream oss;
        oss << EncodedString(parmBox->Text.c_str());
        comm.append(oss.str().c_str());
        comm += "; SETCONFIG\"";
        FileRun1->Parameters = comm.c_str();
    }
    else
    {
        FileRun1->Parameters = "";
    }      */
	//FileRun1->Execute();
	//system(comm.str().c_str());
	STARTUPINFO operatSI,sourceSI, sigSI, appSI;
	PROCESS_INFORMATION operatPI, sourcePI, sigPI, appPI;
	ZeroMemory(&operatSI, sizeof(operatSI));
	operatSI.cb = sizeof(operatSI);
	ZeroMemory(&sourceSI, sizeof(sourceSI));
	sourceSI.cb = sizeof(sourceSI);
	ZeroMemory(&sigSI, sizeof(sigSI));
	sigSI.cb = sizeof(sigSI);
	ZeroMemory(&appSI, sizeof(appSI));
	appSI.cb = sizeof(appSI);

	char *procName = new char[1024];
	strcpy(procName, comm.str().c_str());
	int proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
		HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &operatSI, &operatPI);

	if (!proc)
	{
		statusList->Items->Add(GetLastError());
		return;
	}
   
    //wait a little bit
	Sleep(500);

	comm.str("");  
    if (sourceList->ItemIndex < 0)
    {
        statusList->Items->Add("No Acquisition program selected...");
    }
    else
	{
		statusList->Items->Add("Launching " + sourceList->Items->Strings[sourceList->ItemIndex]);
		comm << "\"" << curdir << sourceList->Items->Strings[sourceList->ItemIndex].c_str() << "\"";
        if (sourceIPBox->Text.Length() > 0)
        {
            comm << " AUTOSTART " << sourceIPBox->Text.c_str();
		}

        if (directoryBox->Text.Length() > 0)
        {
            comm << " --DataDirectory-" << EncodedString(directoryBox->Text.c_str());
        }
        if (subjectNameBox->Text.Length() > 0)
        {
			comm << " --SubjectName-" << EncodedString(subjectNameBox->Text.c_str());
        }
        if (sessionNumBox->Text.Length() > 0)
        {
            comm << " --SubjectSession-" << EncodedString(sessionNumBox->Text.c_str());
		}
		strcpy(procName, comm.str().c_str());
		int proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
			HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &operatSI, &operatPI);

		if (!proc)
		{
			statusList->Items->Add(GetLastError());
			return;
		}
	}

	comm.str("");
    if (sigProcList->ItemIndex < 0)
    {
        statusList->Items->Add("No SigProc program selected...");
    }
    else
    {
		statusList->Items->Add("Launching " + sigProcList->Items->Strings[sigProcList->ItemIndex]);
		comm << "\"" << curdir.c_str() << sigProcList->Items->Strings[sigProcList->ItemIndex].c_str() << "\"";
		if (sigProcIPBox->Text.Length() > 0)
			comm << " AUTOSTART" << sigProcIPBox->Text.c_str();
		strcpy(procName, comm.str().c_str());
		int proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
			HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &operatSI, &operatPI);

		if (!proc)
		{
			statusList->Items->Add(GetLastError());
			return;
		}
	}

	comm.str("");
    if (appList->ItemIndex < 0)
    {
        statusList->Items->Add("No Applications program selected...");
    }
    else
    {
		statusList->Items->Add("Launching " + appList->Items->Strings[appList->ItemIndex]);
        comm << "\"" << curdir.c_str() << appList->Items->Strings[appList->ItemIndex].c_str() << "\"";
        if (appIPBox->Text.Length() > 0)
			comm << " AUTOSTART" << appIPBox->Text.c_str();
		strcpy(procName, comm.str().c_str());
		int proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
			HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &operatSI, &operatPI);

		if (!proc)
		{
			statusList->Items->Add(GetLastError());
			return;
		}
	}

    if (othersList->ItemIndex >= 0)
    {
        //launch other programs as well...
        for (int i =0; i < othersList->Items->Count; i++)
        {
            if (othersList->Selected[i])
			{
				comm << "\"" << curdir << othersList->Items->Strings[i].c_str() << "\"";
				strcpy(procName, comm.str().c_str());
				int proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
					HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &operatSI, &operatPI);

				if (!proc)
				{
					statusList->Items->Add(GetLastError());
					return;
				}
            }
        }
	}
	statusList->Items->Add("Finished!");
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::getParmButClick(TObject */*Sender*/)
{
    OpenParmDlg->FileName = "*.prm";
	OpenParmDlg->DefaultExt = "prm";
	if (OpenParmDlg->Execute())
	{
		for (int i = OpenParmDlg->Files->Count-1;i>=0; i--)
		{
			string tmp = OpenParmDlg->Files->Strings[i].c_str();
            addParm(tmp);
		}
	}
	
}

void TmainForm::addParm(string prm)
{
    parmFiles.push_back(prm);
    int pos = prm.find_last_of("\\");
    //parmList->Lines->Add(tmp);
    parmList->Items->Add(prm.substr(pos+1).c_str());
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::clearButClick(TObject */*Sender*/)
{
    int i=0;
    for (i =0; i < sourceList->Count; i++)
        sourceList->Selected[i] = false;

    for (i =0; i < sigProcList->Count; i++)
        sigProcList->Selected[i] = false;

    for (i =0; i < appList->Count; i++)
        appList->Selected[i] = false;
        
    for (i =0; i < othersList->Count; i++)
        othersList->Selected[i] = false;
}
//---------------------------------------------------------------------------



void __fastcall TmainForm::sourceToSPClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < sourceList->Items->Count)
    {
        if (!sourceList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = sourceList->Items->operator [](i);
        sourceList->Items->Delete(i);
        sigProcList->Items->Insert(0, str);
    }

    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sourceToAppClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < sourceList->Items->Count)
    {
        if (!sourceList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = sourceList->Items->operator [](i);
        sourceList->Items->Delete(i);
        appList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sourceToOthersClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < sourceList->Items->Count)
    {
        if (!sourceList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = sourceList->Items->operator [](i);
        sourceList->Items->Delete(i);
        othersList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::spToSourceClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < sigProcList->Items->Count)
    {
        if (!sigProcList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = sigProcList->Items->operator [](i);
        sigProcList->Items->Delete(i);
        sourceList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::spToAppClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < sigProcList->Items->Count)
    {
        if (!sigProcList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = sigProcList->Items->operator [](i);
        sigProcList->Items->Delete(i);
        appList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::spToOthersClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < sigProcList->Items->Count)
    {
        if (!sigProcList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = sigProcList->Items->operator [](i);
        sigProcList->Items->Delete(i);
        othersList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appToSourceClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < appList->Items->Count)
    {
        if (!appList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = appList->Items->operator [](i);
        appList->Items->Delete(i);
        sourceList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appToSpClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < appList->Items->Count)
    {
        if (!appList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = appList->Items->operator [](i);
        appList->Items->Delete(i);
        sigProcList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appToOthersClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < appList->Items->Count)
    {
        if (!appList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = appList->Items->operator [](i);
        appList->Items->Delete(i);
        othersList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::othersToSourceClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < othersList->Items->Count)
    {
        if (!othersList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = othersList->Items->operator [](i);
        othersList->Items->Delete(i);
        sourceList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::othersToSpClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < othersList->Items->Count)
    {
        if (!othersList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = othersList->Items->operator [](i);
        othersList->Items->Delete(i);
        sigProcList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::othersToAppClick(TObject */*Sender*/)
{
    //this loop finds all of the items selected, and moves them to the correct
    //list. Only one item can actually be selected, but this is included to avoid
    //future bugs...
    int i = 0;
    AnsiString str;
    while (i < othersList->Items->Count)
    {
        if (!othersList->Selected[i])
        {
            i++;
            continue;
        }

        // it is selected, so we need to remove it and put it in the SP list
        str = othersList->Items->operator [](i);
        othersList->Items->Delete(i);
        appList->Items->Insert(0, str);
    }
    updateINIFile();
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::sourceListDragDrop(TObject */*Sender*/,
      TObject */*Source*/, int X, int Y)
{
    TPoint dropPoint;
    int dropPos, startPos;

    dropPoint.x = X;
    dropPoint.y = Y;

    startPos = sourceList->ItemAtPos(startPoint, true);
    dropPos = sourceList->ItemAtPos(dropPoint, true);
    sourceList->Items->Move(startPos, dropPos);
    sourceList->Selected[dropPos] = true;

    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sourceListDragOver(TObject */*Sender*/,
      TObject *Source, int /*X*/, int /*Y*/, TDragState /*State*/, bool &Accept)
{
    Accept = (Source == sourceList);
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sourceListMouseDown(TObject */*Sender*/,
      TMouseButton /*Button*/, TShiftState /*Shift*/, int X, int Y)
{
    startPoint.x = X;
    startPoint.y = Y;
}
//---------------------------------------------------------------------------

bool TmainForm::updateINIFile()
{
    //first, open the file for writing, and erase contents in the process
    ofstream out;
    int i;
    out.open(iniFile.c_str(), ios::out);

    if (out.fail())
        return false;

    //now go through the source, sigProc, and app lists and output the names of each
    for (i = 0; i < sourceList->Items->Count; i++)
        out << "source " << sourceList->Items->operator [](i).c_str() << endl;

    out << endl;

    for (i = 0; i < sigProcList->Items->Count; i++)
        out << "sigproc " << sigProcList->Items->operator [](i).c_str() << endl;

    out <<endl;

    for (i = 0; i < appList->Items->Count; i++)
        out << "app " << appList->Items->operator [](i).c_str() << endl;

    out.close();
    return true;

}
void __fastcall TmainForm::FormClose(TObject */*Sender*/, TCloseAction &/*Action*/)
{
    updateINIFile();    
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sigProcListDragDrop(TObject */*Sender*/,
      TObject */*Source*/, int X, int Y)
{
    TPoint dropPoint;
    int dropPos, startPos;

    dropPoint.x = X;
    dropPoint.y = Y;

    startPos = sigProcList->ItemAtPos(startPoint, true);
    dropPos = sigProcList->ItemAtPos(dropPoint, true);
    sigProcList->Items->Move(startPos, dropPos);
    sigProcList->Selected[dropPos] = true;

    updateINIFile();
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sigProcListDragOver(TObject */*Sender*/,
      TObject *Source, int /*X*/, int /*Y*/, TDragState /*State*/, bool &Accept)
{
    Accept = (Source == sigProcList);
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sigProcListMouseDown(TObject */*Sender*/,
      TMouseButton /*Button*/, TShiftState /*Shift*/, int X, int Y)
{
    startPoint.x = X;
    startPoint.y = Y;
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::appListDragOver(TObject */*Sender*/,
      TObject *Source, int /*X*/, int /*Y*/, TDragState /*State*/, bool &Accept)
{
    Accept = (Source == appList);
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appListMouseDown(TObject */*Sender*/,
      TMouseButton /*Button*/, TShiftState /*Shift*/, int X, int Y)
{
    startPoint.x = X;
    startPoint.y = Y;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appListDragDrop(TObject */*Sender*/,
      TObject */*Source*/, int X, int Y)
{
    TPoint dropPoint;
    int dropPos, startPos;

    dropPoint.x = X;
    dropPoint.y = Y;

    startPos = appList->ItemAtPos(startPoint, true);
    dropPos = appList->ItemAtPos(dropPoint, true);
    appList->Items->Move(startPos, dropPos);
    appList->Selected[dropPos] = true;

    updateINIFile();
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::helpMnuClick(
      TObject */*Sender*/)
{
    ExecutableHelp().Display();
/*    if (access(helpLoc.c_str(), 0) != 0)
    {
        int ans = Application->MessageBoxA("The help file could not be found. Do you want to specify the file location yourself?","Missing Help File",MB_YESNO);
        if (ans == IDNO)
            return;

        OpenParmDlg->FileName = "BCIlauncher_help.chm";
        OpenParmDlg->Filter = "Help Files (*.chm)|*.CHM";
        if (OpenParmDlg->Execute())
        {
            helpLoc = OpenParmDlg->FileName;
            FileRun1->FileName = OpenParmDlg->FileName;
            FileRun1->Execute();
            return;
        }
    }
    else
    {
        FileRun1->FileName = helpLoc;
        FileRun1->Execute();
    } */
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::delParmButClick(TObject */*Sender*/)
{
	int i = 0;
	vector<string>::iterator it = parmFiles.begin();
	while (i < parmList->Count)
	{
		if (parmList->Selected[i])
		{
			parmList->Items->Delete(i);
			parmFiles.erase(it);
		}
		else
		{
			i++;
			it++;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::About1Click(TObject */*Sender*/)
{
    AboutBox().SetApplicationName( "BCI2000Launcher" )
            .Display();    
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::getDirBtnClick(TObject */*Sender*/)
{
	VclStringType Dir = "";
  if (SelectDirectory(Dir, TSelectDirOpts() << sdAllowCreate << sdPerformCreate << sdPrompt,SELDIRHELP))
	directoryBox->Text = Dir;
    
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::sessionNumBoxChange(TObject */*Sender*/)
{               /*
    if (sessionNumBox->Text.Length() == 0)
        return;
    if (sessionNumBox->Text.Length() < 3)
    {
        AnsiString tmp;
        for (int i=0; i < 3-sessionNumBox->Text.Length(); i++)
            tmp << "0";
        for (int i=0; i < sessionNumBox->Text.Length(); i++)
            tmp << sessionNumBox->Text[i];
        sessionNumBox->Text = tmp;
        return;
    }
    if (sessionNumBox->Text.Length() > 3)
    {
        AnsiString tmp;
        for (int i=0; i < 3; i++)
            tmp << sessionNumBox->Text[i];
        sessionNumBox->Text = tmp;
        return;
    }
    return;   */
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sessionNumBoxExit(TObject */*Sender*/)
{
    if (sessionNumBox->Text.Length() == 0)
        return;
    if (sessionNumBox->Text.Length() < 3)
    {
        string tmp = "";
        for (int i=0; i < 3-sessionNumBox->Text.Length(); i++)
            tmp += "0";
        tmp += sessionNumBox->Text.c_str();
        sessionNumBox->Text = tmp.c_str();
        return;
    }
    if (sessionNumBox->Text.Length() > 3)
    {
        AnsiString tmp = sessionNumBox->Text.SubString(0,3);
        sessionNumBox->Text = tmp;
        return;
    }
    return;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sessionNumBoxKeyPress(TObject */*Sender*/,
      char &Key)
{
    if (Key ==13)
    {
        if (sessionNumBox->Text.Length() == 0)
            return;
        if (sessionNumBox->Text.Length() < 3)
        {
            string tmp = "";
            for (int i=0; i < 3-sessionNumBox->Text.Length(); i++)
                tmp += "0";
            tmp += sessionNumBox->Text.c_str();
            sessionNumBox->Text = tmp.c_str();
            return;
        }
        if (sessionNumBox->Text.Length() > 3)
        {
            AnsiString tmp = sessionNumBox->Text.SubString(0,3);
            sessionNumBox->Text = tmp;
            return;
        }
        return;
    }    
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::SIGFRIED1Click(TObject */*Sender*/)
{
    if (sourceList->ItemIndex < 0)
    {
        statusList->Items->Add("No Acquisition program selected...");
    }
    string sigSource = sourceList->Items->Strings[sourceList->ItemIndex].c_str();
    sigfriedUI = new TSigfried_UIfrm(this, sigSource, progDir);
    sigfriedUI->ShowModal();
    if (sigfriedUI->Status() == 1)
    {
        string tmp = sigfriedUI->ParmFile();
        addParm(tmp);
    }
}
//---------------------------------------------------------------------------


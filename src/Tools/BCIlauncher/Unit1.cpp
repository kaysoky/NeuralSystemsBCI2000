//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <stdio.h>
#include <io.h>
#include <iostream>
#include <fstream>

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

    helpLoc = "..\\doc\\BCIlauncher_help.chm";
}
//---------------------------------------------------------------------------




void __fastcall TmainForm::FormCreate(TObject *Sender)
{
    //lists of programs
    bool haveFile = false;
    int boxDlg;
    vector<string> lines;
    string label, value;
    ifstream in;
    in.open("BCIlauncher.ini");

    if (!in.fail())
        haveFile = true;
    if (in.fail())
    {
        in.close();
        in.clear();
        boxDlg = Application->MessageBoxA("The BCIlauncher.ini file cannot be found, so a new one will be created in the BCI2000/prog folder. Press Ignore, and move the programs from the Others list to the appropriate list (a new ini file will be created), or press Retry to select a file.",
            "*.ini file not found",MB_ABORTRETRYIGNORE);

        switch (boxDlg)
        {
            case IDABORT:
                exit(0);
                break;
            case IDRETRY:
                //get a new *.ini file
                OpenParmDlg->FileName = "BCIlauncher.ini";
                OpenParmDlg->Filter = "*.ini";
                if (OpenParmDlg->Execute())
                {
                    in.open(OpenParmDlg->FileName.c_str());
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
                Application->MessageBoxA("The BCIlauncher.ini file is corrupted, or an invalid file was selected.","Invalid INI file",MB_OK);
            }
            lines.clear();
        }
        in.close();
    }

    //start by getting the directory listing
    struct ffblk ffblk;
    int done, count;
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
    for (int i = 0; i < sourceStr.size(); i++)
    {
        if (ismember(sourceStr[i], dirListing))
            sourceList->Items->Add(sourceStr[i].c_str());
    }

    for (int i = 0; i < SPStr.size(); i++)
    {
        if (ismember(SPStr[i], dirListing))
            sigProcList->Items->Add(SPStr[i].c_str());
    }

    for (int i = 0; i < appStr.size(); i++)
    {
        if (ismember(appStr[i], dirListing))
            appList->Items->Add(appStr[i].c_str());
    }

    count = 0;
    while (count < dirListing.size())
    {
        if ((!ismember(dirListing[count], appStr)) && (!ismember(dirListing[count], SPStr)) && (!ismember(dirListing[count], sourceStr)))
        {
            if (!(dirListing[count] == "operat.exe") && !(dirListing[count] == "BCIlauncher.exe"))
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
    for (int i=0; i < strs.size(); i++)
    {
        //if (strcmp(str.c_str(), strs[i].c_str()) == 0)
        if (str == strs[i])
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------
void removeAt(vector<string> &str, int pos)
{
    if (pos >= str.size() || pos < 0)
        return;

    vector<string> newStr = str;
    str.clear();
    for (int i=0; i < newStr.size(); i++)
    {
        if (i != pos)
            str.push_back(newStr[i]);
    }
}

void __fastcall TmainForm::launchButClick(TObject *Sender)
{
    statusList->Clear();
    statusList->Items->Add("Launching Operator...");
    FileRun1->FileName = "operat.exe";
    FileRun1->Execute();

    //wait a little bit
    for (int i = 0; i < 100000; i+=2)
        i--;

    if (sourceList->ItemIndex < 0)
    {
        statusList->Items->Add("No Acquisition program selected...");
    }
    else
    {
        statusList->Items->Add("Launching " + sourceList->Items->Strings[sourceList->ItemIndex]);
        FileRun1->FileName = sourceList->Items->Strings[sourceList->ItemIndex];
        if (sourceIPBox->Text.Length() > 0)
            FileRun1->Parameters = "AUTOSTART " + sourceIPBox->Text;
        else
            FileRun1->Parameters = "";
            
        FileRun1->Execute();
    }

    if (sigProcList->ItemIndex < 0)
    {
        statusList->Items->Add("No SigProc program selected...");
    }
    else
    {
        statusList->Items->Add("Launching " + sigProcList->Items->Strings[sigProcList->ItemIndex]);
        FileRun1->FileName = sigProcList->Items->Strings[sigProcList->ItemIndex];
        if (sigProcIPBox->Text.Length() > 0)
            FileRun1->Parameters = "AUTOSTART " + sigProcIPBox->Text;
        else
            FileRun1->Parameters = "";
        FileRun1->Execute();
    }

    if (appList->ItemIndex < 0)
    {
        statusList->Items->Add("No Applications program selected...");
    }
    else
    {
        statusList->Items->Add("Launching " + appList->Items->Strings[appList->ItemIndex]);
        FileRun1->FileName = appList->Items->Strings[appList->ItemIndex];
        if (appIPBox->Text.Length() > 0)
            FileRun1->Parameters = "AUTOSTART " + appIPBox->Text;
        else
            FileRun1->Parameters = "";
            
        FileRun1->Execute();
    }

    if (othersList->ItemIndex >= 0)
    {
        //launch other programs as well...
        for (int i =0; i < othersList->Items->Count; i++)
        {
            if (othersList->Selected[i])
            {
                FileRun1->FileName = othersList->Items->Strings[i];
                FileRun1->Parameters = "";
                FileRun1->Execute();
            }
        }
    }
    statusList->Items->Add("Finished!");
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::getParmButClick(TObject *Sender)
{
    OpenParmDlg->FileName = "*.prm";
    OpenParmDlg->DefaultExt = "prm";
    if (OpenParmDlg->Execute())
    {
        parmBox->Text = OpenParmDlg->FileName;
    }
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::clearButClick(TObject *Sender)
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

bool getNextLine(ifstream &in, vector<string> &tokens, string delimiters)
{
    bool found = false;
    int pos;
    string line;

    if (in.eof())
        return false;
        
    while (!found && !in.eof())
    {
        tokens.clear();
        line.erase();

        
        if (in.eof())
            return false;

        //in.getline(line, 500, '\n');
       // getline(in, line, '\n');
       getline(in, line);

       //remove comments
       pos = line.find("//");
       if (pos != string::npos)
       {
            line.erase(pos);
       }

       //remove leading spaces or tabs
       pos = line.find_first_not_of(" \t");
		if (pos != string::npos)
		{
			//cout << "erasing leading spaces..."<<endl;
			line.erase(0, pos);
		}

		// remove trailing spaces
		pos = line.find_last_not_of(" \t");
		if (pos != string::npos)
		{
			
			//cout <<"erasing trailing spaces..."<<endl;
			//cout << "Pos = "<<pos<<endl;
			line.erase(pos+1);
		}

		if (line.length() < 1)
		{
			//cout <<"going to next line..."<<endl;
			// get the next line
			continue;
		}
        if (line.length() == 1)
		{
			//cout <<"LENGTH = 1. Char is "<<(int)line[0]<<endl;
		}

		if (line.length() < 1)
		{
			//cout <<"Nothing left, going to next line..."<<endl;
			// get next line
			continue;
		}
        stringSplit(line, tokens, delimiters);
		//getchar();
		found = true;
    }
}

void stringSplit(const string& str, vector<string>& tokens, string delimiters)
{

	tokens.clear();
	// Find the first position in the string that is not a delimiter
	int start = str.find_first_not_of(delimiters);
	int endStr = 0;

	//cout <<"String: ["<<str<<"]"<<endl;
	//cout <<"Start: "<<start<<endl;
	
	while (start != string::npos)
	{
		endStr = str.find_first_of(delimiters, start+1);
		if (endStr == string::npos)
			endStr = str.length();

		tokens.insert(tokens.end(), str.substr(start, endStr - start));

		start = str.find_first_not_of(delimiters, endStr+1);
		//cout << str.substr(start, endStr - start) <<endl;
	}
	

	return;
}

string lowerCase(string str)
{
	string returnStr = str;
	for (int i = 0; i < returnStr.length(); i++)
	{
		returnStr[i] = tolower(returnStr[i]);
	}
	return returnStr;
}



void __fastcall TmainForm::sourceToSPClick(TObject *Sender)
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

void __fastcall TmainForm::sourceToAppClick(TObject *Sender)
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

void __fastcall TmainForm::sourceToOthersClick(TObject *Sender)
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

void __fastcall TmainForm::spToSourceClick(TObject *Sender)
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

void __fastcall TmainForm::spToAppClick(TObject *Sender)
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

void __fastcall TmainForm::spToOthersClick(TObject *Sender)
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

void __fastcall TmainForm::appToSourceClick(TObject *Sender)
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

void __fastcall TmainForm::appToSpClick(TObject *Sender)
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

void __fastcall TmainForm::appToOthersClick(TObject *Sender)
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

void __fastcall TmainForm::othersToSourceClick(TObject *Sender)
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

void __fastcall TmainForm::othersToSpClick(TObject *Sender)
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

void __fastcall TmainForm::othersToAppClick(TObject *Sender)
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


void __fastcall TmainForm::sourceListDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
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

void __fastcall TmainForm::sourceListDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Accept = Source = sourceList;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sourceListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
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
    out.open("BCIlauncher.ini", ios::out);

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
void __fastcall TmainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    updateINIFile();    
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sigProcListDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
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

void __fastcall TmainForm::sigProcListDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Accept = Source = sigProcList;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::sigProcListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    startPoint.x = X;
    startPoint.y = Y;
}
//---------------------------------------------------------------------------


void __fastcall TmainForm::appListDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Accept = Source = appList;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    startPoint.x = X;
    startPoint.y = Y;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::appListDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
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
      TObject *Sender)
{
    if (access(helpLoc.c_str(), 0) != 0)
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
    }
}
//---------------------------------------------------------------------------



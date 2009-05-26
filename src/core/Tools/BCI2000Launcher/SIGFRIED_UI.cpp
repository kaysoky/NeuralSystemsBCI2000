//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SIGFRIED_UI.h"
#include "VCLDefines.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSigfried_UIfrm *Sigfried_UIfrm;
//---------------------------------------------------------------------------
__fastcall TSigfried_UIfrm::TSigfried_UIfrm(TComponent* Owner, string sigSource, string progDir)
    : TForm(Owner)
{
    mSigSource = sigSource;
    mStatus = 0;
    mProgDir = progDir;
    string caption = "";
    caption += "SIGFRIED Startup: ";
    caption += mSigSource;
    this->Caption = caption.c_str();
    mRecordReady = mModelReady= false;
    models.clear();
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::getDirBtnClick(TObject */*Sender*/)
{
    VclStringType Dir = "";
    if (SelectDirectory(Dir, TSelectDirOpts() << sdAllowCreate << sdPerformCreate << sdPrompt,1000))
        directoryBox->Text = Dir;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::sessionNumBoxExit(TObject */*Sender*/)
{
    if (sessionNumBox->Text.Length() == 0)
    {
        updateRecordReady();
        return;
    }
    if (sessionNumBox->Text.Length() < 3)
    {
        string tmp = "";
        for (int i=0; i < 3-sessionNumBox->Text.Length(); i++)
            tmp += "0";
        tmp += AnsiString(sessionNumBox->Text).c_str();
        sessionNumBox->Text = tmp.c_str();
    }
    if (sessionNumBox->Text.Length() > 3)
    {
        AnsiString tmp = sessionNumBox->Text.SubString(0,3);
        sessionNumBox->Text = tmp;
    }
    updateRecordReady();
    return;    
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::sessionNumBoxKeyPress(TObject */*Sender*/,
      char &Key)
{
    if (Key ==13)
    {
        if (sessionNumBox->Text.Length() == 0)
        {
            updateRecordReady();
            return;
        }
        if (sessionNumBox->Text.Length() < 3)
        {
            string tmp = "";
            for (int i=0; i < 3-sessionNumBox->Text.Length(); i++)
                tmp += "0";
            tmp += AnsiString(sessionNumBox->Text).c_str();
            sessionNumBox->Text = tmp.c_str();
        }
        if (sessionNumBox->Text.Length() > 3)
        {
            AnsiString tmp = sessionNumBox->Text.SubString(0,3);
            sessionNumBox->Text = tmp;
        }
        updateRecordReady();
    }      
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::baselinePrmBtnClick(TObject */*Sender*/)
{
    OpenFileDlg->FileName = "";
    OpenFileDlg->Filter = "PRM Files(*.prm)|*.prm";
    if (OpenFileDlg->Execute())
    {
		AnsiString name = OpenFileDlg->FileName;
		mParmFile = name.c_str();
        baselinePrmBox->Text = mParmFile.c_str();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::directoryBoxChange(TObject */*Sender*/)
{
    updateRecordReady();
}

void TSigfried_UIfrm::updateRecordReady()
{                                                 
    mRecordReady = (sessionNumBox->Text.Length() > 0) &&
                        (baselinePrmBox->Text.Length() > 0) &&
                        (directoryBox->Text.Length() > 0);

    recordBaselineBtn->Enabled = mRecordReady;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::baselinePrmBoxChange(TObject */*Sender*/)
{
    updateRecordReady();    
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::recordBaselineBtnClick(TObject */*Sender*/)
{
    //check here if a dat file already exists in the current location
    //check if dat file exists, warn, and backup
    //check if model exists, warn, and backup
    //-----------------

    //we need a temporary parameter fragment that contains
    //the datadirectory, session #, and subject name (baseline)
    ofstream of;
    string fragTmp;
    fragTmp = mProgDir + "\\..\\parms\\fragTmp.prm";
	of.open(fragTmp.c_str(), ios::out);
	stringstream comm;
    if (of.fail())
    {
    }

    //write the parameter information
    of <<"Storage:Data%20Location:DataIOFilter string DataDirectory= ";
    of << EncodedString(AnsiString(directoryBox->Text).c_str()) << " ..\\data % % //"<<endl;

    of <<"Storage:Session:DataIOFilter string SubjectSession= ";
    of << sessionNumBox->Text.c_str() << " 001 % % //" <<endl;

    of <<"Storage:Session:DataIOFilter string SubjectName= baseline name % % //" << endl;

    of.close();

	//now start the recording session based on the parameters passed

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

	comm << mProgDir << "\\operat.exe --OnConnect \" LOAD PARAMETERFILE "<< EncodedString(AnsiString(baselinePrmBox->Text).c_str()) << "; ";
	comm << "LOAD PARAMETERFILE " <<EncodedString(fragTmp) << ";  SETCONFIG;\"";

	char *procName = new char[1024];
	strcpy(procName, comm.str().c_str());
	int proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
		HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &operatSI, &operatPI);

	if (!proc)
	{
		return;
	}


    //wait a little bit
    Sleep(500);

    //now, launch the provided sigsource, and dummysignalprocessing and stimulus presentation
	comm.str("");
	comm << mProgDir << "\\" << mSigSource << "AUTOSTART 127.0.0.7";
	strcpy(procName, comm.str().c_str());
	proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
		HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &sourceSI, &sourcePI);

	if (!proc)
	{
		return;
	}

	comm.str("");
	comm << mProgDir << "\\dummysignalprocessing.exe AUTOSTART 127.0.0.7";
	strcpy(procName, comm.str().c_str());
	 proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
		HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &sourceSI, &sourcePI);

	if (!proc)
	{
		return;
	}

    comm.str("");
	comm << mProgDir <<"\\StimulusPresentation.exe AUTOSTART 127.0.0.7";
	strcpy(procName, comm.str().c_str());
	proc = CreateProcess(NULL,procName, NULL, NULL, FALSE,
		HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &sigSI, &sigPI);

	if (!proc)
	{
		return;
	}

    mBaselineFile = EncodedString(AnsiString(directoryBox->Text).c_str()) + "\\baseline";
    mBaselineFile += string(AnsiString(sessionNumBox->Text).c_str()) + "\\";
    mBaselineFile += "baselineS" + string(AnsiString(sessionNumBox->Text).c_str()) + "R01.dat";
    baselineFileBox->Text = mBaselineFile.c_str();

    string tmpModel = EncodedString(AnsiString(directoryBox->Text).c_str()) + "\\baseline";
    tmpModel += string(AnsiString(sessionNumBox->Text).c_str()) + "\\model.mdl";
    modelFileBox->Text = tmpModel.c_str();
    
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelIniBtnClick(TObject */*Sender*/)
{
    OpenFileDlg->FileName = "";
    OpenFileDlg->Filter = "INI files(*.ini)|*.ini";
    if (OpenFileDlg->Execute())
    {
        mIniFile = AnsiString(OpenFileDlg->FileName).c_str();
        modelIniBox->Text = mIniFile.c_str();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelDirBtnClick(TObject */*Sender*/)
{
    SaveFileDlg->DefaultExt = "mdl";
    SaveFileDlg->FileName = modelFileBox->Text;
    SaveFileDlg->Filter = "Sigfried Model files(*.mdl)|*.mdl";

    if (SaveFileDlg->Execute())
    {
        mModelFile = AnsiString(SaveFileDlg->FileName).c_str();
        modelFileBox->Text = mModelFile.c_str();
    }

}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelFileBoxChange(TObject */*Sender*/)
{
    mModelFile = AnsiString(modelFileBox->Text).c_str();
    mModelReady = (mModelFile.size() > 0) && (mIniFile.size() > 0 && mBaselineFile.size() > 0);
    buildModelBtn->Enabled = mModelReady;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelIniBoxChange(TObject */*Sender*/)
{
    mIniFile = AnsiString(modelIniBox->Text).c_str();
    mModelReady = (mModelFile.size() > 0) && (mIniFile.size() > 0 && mBaselineFile.size() > 0);
    buildModelBtn->Enabled = mModelReady;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::buildModelBtnClick(TObject */*Sender*/)
{
    for (size_t i = 0; i < models.size(); i++)
    {
		stringstream comm;
		comm << mProgDir << "\\data2model_gui.exe ";

        //mBaselineFile = "baseline.dat";
		comm << "-inicfg " + models[i].iniFile;
		comm << " -inisig " + mProgDir + "\\sigfried.ini";
		comm << " -input " + mBaselineFile;
        comm << " -output " + models[i].modelOutput;

		char *procName = new char[1024];
		strcpy(procName, comm.str().c_str());
		/*int proc =*/ CreateProcess(NULL,procName, NULL, NULL, FALSE,
			HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, NULL, NULL);
    }
}

//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::addModelBtnClick(TObject */*Sender*/)
{
    if (modelIniBox->Text.Length() == 0 ||
        modelFileBox->Text.Length() == 0 ||
        modelDescBox->Text.Length() == 0)
        return;

    Model tmpModel;
    tmpModel.iniFile = AnsiString(modelIniBox->Text).c_str();
	tmpModel.modelOutput = AnsiString(modelFileBox->Text).c_str();
	tmpModel.description = AnsiString(modelDescBox->Text).c_str();
    models.push_back(tmpModel);
    updateModelList();
}
//---------------------------------------------------------------------------

void TSigfried_UIfrm::updateModelList()
{
    modelList->Clear();
    for (size_t i = 0; i < models.size(); i++)
    {
        modelList->Items->Add(models[i].description.c_str());
    }
}
void __fastcall TSigfried_UIfrm::remModelBtnClick(TObject */*Sender*/)
{
    int i = 0;
	vector<Model>::iterator it = models.begin();
	while (i < modelList->Count)
	{
		if (modelList->Selected[i])
		{
			modelList->Items->Delete(i);
			models.erase(it);
		}
		else
		{
			i++;
			it++;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::parmOutputBtnClick(TObject */*Sender*/)
{
    SaveFileDlg->DefaultExt = "prm";
    SaveFileDlg->FileName = modelFileBox->Text;
    SaveFileDlg->Filter = "BCI2000 prm file (*.prm)|*.prm";

    if (SaveFileDlg->Execute())
    {
        mParmOutputFile = AnsiString( SaveFileDlg->FileName ).c_str();
        parmOutputBox->Text = mParmOutputFile.c_str();
    }
    
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::parmOutputBoxChange(TObject */*Sender*/)
{
    returnBtn->Enabled = (parmOutputBox->Text.Length() > 0);    
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::returnBtnClick(TObject */*Sender*/)
{
	int width = atoi(AnsiString(visModelWidthBox->Text).c_str());
    int height = atoi(AnsiString(visModelHeightBox->Text).c_str());
    bool visModels = visModelsCheck->Checked;
    
    ofstream of;
    //string fragTmp;
    //fragTmp = mProgDir + "\\..\\parms\\fragTmp.prm";
    of.open(mParmOutputFile.c_str(), ios::out);
    int curXPos = 0;
    if (of.fail())
    {
        return;
    }
    //Filtering:SigfriedARFilter matrix ModelFiles= 1 { filename label x-pos y-pos width }
    // ../model/baseline.mdl mu/beta -2 516 800 // model files
    //write the parameter information
    of << "Visualize:SigfriedARFilter int VisualizeSigfried= ";
    if (visModels) of << "1";
    else of << "0";
    of << " 0 0 1 //"<<endl;
    
    of << "Filtering:SigfriedARFilter matrix ModelFiles= " << models.size();
    of << " { filename label x-pos y-pos width } ";

    for (size_t i = 0; i < models.size(); i++)
    {
        of << EncodedString(models[i].modelOutput) << " ";
        of << EncodedString(models[i].description) << " ";
        of << curXPos << " " << height << " " << width << " ";
        curXPos += width;
    }

    of << "//"<<endl;
    of.close();
    mStatus = 1;
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::getBaselineFileBtnClick(TObject */*Sender*/)
{
    OpenFileDlg->FileName = "";
    OpenFileDlg->Filter = "BCI2000 Dat file(*.dat)|*.dat";
    if (OpenFileDlg->Execute())
    {
        mBaselineFile = AnsiString( OpenFileDlg->FileName ).c_str();
        baselineFileBox->Text = mBaselineFile.c_str();
    }
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::baselineFileBoxChange(TObject */*Sender*/)
{
    mBaselineFile = AnsiString(baselineFileBox->Text).c_str();
    mModelReady = (mModelFile.size() > 0) && (mIniFile.size() > 0 && mBaselineFile.size() > 0);
    buildModelBtn->Enabled = mModelReady;
}
//---------------------------------------------------------------------------


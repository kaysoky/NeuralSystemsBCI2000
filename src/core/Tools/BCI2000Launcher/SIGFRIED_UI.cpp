//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SIGFRIED_UI.h"
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
void __fastcall TSigfried_UIfrm::getDirBtnClick(TObject *Sender)
{
    AnsiString Dir = "";
    if (SelectDirectory(Dir, TSelectDirOpts() << sdAllowCreate << sdPerformCreate << sdPrompt,1000))
        directoryBox->Text = Dir;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::sessionNumBoxExit(TObject *Sender)
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
        tmp += sessionNumBox->Text.c_str();
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
void __fastcall TSigfried_UIfrm::sessionNumBoxKeyPress(TObject *Sender,
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
            tmp += sessionNumBox->Text.c_str();
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
void __fastcall TSigfried_UIfrm::baselinePrmBtnClick(TObject *Sender)
{
    OpenFileDlg->FileName = "";
    OpenFileDlg->Filter = "PRM Files(*.prm)|*.prm";
    if (OpenFileDlg->Execute())
    {
        mParmFile = OpenFileDlg->FileName.c_str();
        baselinePrmBox->Text = mParmFile.c_str();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::directoryBoxChange(TObject *Sender)
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
void __fastcall TSigfried_UIfrm::baselinePrmBoxChange(TObject *Sender)
{
    updateRecordReady();    
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::recordBaselineBtnClick(TObject *Sender)
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
    if (of.fail())
    {
    }

    //write the parameter information
    of <<"Storage:Data%20Location:DataIOFilter string DataDirectory= ";
    of << EncodedString(directoryBox->Text.c_str()) << " ..\\data % % //"<<endl;

    of <<"Storage:Session:DataIOFilter string SubjectSession= ";
    of << sessionNumBox->Text.c_str() << " 001 % % //" <<endl;

    of <<"Storage:Session:DataIOFilter string SubjectName= baseline name % % //" << endl;

    of.close();

    //now start the recording session based on the parameters passed
    string comm = "";
    FileRun1->FileName = mProgDir.c_str();
	FileRun1->FileName.Insert("\\operat.exe", mProgDir.length()+1);

    comm = "--OnConnect \"-";
    comm += "LOAD PARAMETERFILE ";
    comm += EncodedString(baselinePrmBox->Text.c_str()) + "; ";
    comm += "LOAD PARAMETERFILE " + EncodedString(fragTmp) + "; ";

    comm +=" SETCONFIG;\"";

    FileRun1->Parameters = comm.c_str();

    FileRun1->Execute();

    //wait a little bit
    Sleep(500);

    //now, launch the provided sigsource, and dummysignalprocessing and stimulus presentation
    comm = "";
    FileRun1->FileName = string(mProgDir + "\\" + mSigSource).c_str();
    comm += "AUTOSTART 127.0.0.7";
    FileRun1->Parameters = comm.c_str();
    FileRun1->Execute();

    comm = "";
    FileRun1->FileName = string(mProgDir + "\\dummysignalprocessing.exe").c_str();
    comm += "AUTOSTART 127.0.0.7";
    FileRun1->Parameters = comm.c_str();
    FileRun1->Execute();

    comm = "";
    FileRun1->FileName = string(mProgDir + "\\StimulusPresentation.exe").c_str();
    comm += "AUTOSTART 127.0.0.7";
    FileRun1->Parameters = comm.c_str();
    FileRun1->Execute();

    mBaselineFile = EncodedString(directoryBox->Text.c_str()) + "\\baseline";
    mBaselineFile += string(sessionNumBox->Text.c_str()) + "\\";
    mBaselineFile += "baselineS" + string(sessionNumBox->Text.c_str()) + "R01.dat";

    string tmpModel = EncodedString(directoryBox->Text.c_str()) + "\\baseline";
    tmpModel += string(sessionNumBox->Text.c_str()) + "\\model.mdl";
    modelFileBox->Text = tmpModel.c_str();
    
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelIniBtnClick(TObject *Sender)
{
    OpenFileDlg->FileName = "";
    OpenFileDlg->Filter = "INI files(*.ini)|*.ini";
    if (OpenFileDlg->Execute())
    {
        mIniFile = OpenFileDlg->FileName.c_str();
        modelIniBox->Text = mIniFile.c_str();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelDirBtnClick(TObject *Sender)
{
    SaveFileDlg->DefaultExt = "mdl";
    SaveFileDlg->FileName = modelFileBox->Text;
    SaveFileDlg->Filter = "Sigfried Model files(*.mdl)|*.mdl";

    if (SaveFileDlg->Execute())
    {
        mModelFile = SaveFileDlg->FileName.c_str();
        modelFileBox->Text = mModelFile.c_str();
    }

}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelFileBoxChange(TObject *Sender)
{
    mModelFile = modelFileBox->Text.c_str();
    mModelReady = (mModelFile.size() > 0) && (mIniFile.size() > 0);
    buildModelBtn->Enabled = mModelReady;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::modelIniBoxChange(TObject *Sender)
{
    mIniFile = modelIniBox->Text.c_str();
    mModelReady = (mModelFile.size() > 0) && (mIniFile.size() > 0);
    buildModelBtn->Enabled = mModelReady;
}
//---------------------------------------------------------------------------
void __fastcall TSigfried_UIfrm::buildModelBtnClick(TObject *Sender)
{
    for (int i = 0; i < models.size(); i++)
    {
        string comm = "";
        FileRun1->FileName = string(mProgDir + "\\data2model_gui.exe").c_str();

        //mBaselineFile = "baseline.dat";
        comm += "-inicfg " + models[i].iniFile;
        comm += " -inisig " + mProgDir + "\\sigfried.ini";
        comm += " -input " + mBaselineFile;
        comm += " -output " + models[i].modelOutput;

        FileRun1->Parameters = comm.c_str();
        FileRun1->Execute();
    }
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::addModelBtnClick(TObject *Sender)
{
    if (modelIniBox->Text.Length() == 0 ||
        modelFileBox->Text.Length() == 0 ||
        modelDescBox->Text.Length() == 0)
        return;

    Model tmpModel;
    tmpModel.iniFile = modelIniBox->Text.c_str();
    tmpModel.modelOutput = modelFileBox->Text.c_str();
    tmpModel.description = modelDescBox->Text.c_str();
    models.push_back(tmpModel);
    updateModelList();
}
//---------------------------------------------------------------------------

void TSigfried_UIfrm::updateModelList()
{
    modelList->Clear();
    for (int i = 0; i < models.size(); i++)
    {
        modelList->Items->Add(models[i].description.c_str());
    }
}
void __fastcall TSigfried_UIfrm::remModelBtnClick(TObject *Sender)
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

void __fastcall TSigfried_UIfrm::parmOutputBtnClick(TObject *Sender)
{
    SaveFileDlg->DefaultExt = "prm";
    SaveFileDlg->FileName = modelFileBox->Text;
    SaveFileDlg->Filter = "BCI2000 prm file (*.prm)|*.prm";

    if (SaveFileDlg->Execute())
    {
        mParmOutputFile = SaveFileDlg->FileName.c_str();
        parmOutputBox->Text = mParmOutputFile.c_str();
    }
    
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::parmOutputBoxChange(TObject *Sender)
{
    returnBtn->Enabled = (parmOutputBox->Text.Length() > 0);    
}
//---------------------------------------------------------------------------

void __fastcall TSigfried_UIfrm::returnBtnClick(TObject *Sender)
{
    int width = atoi(visModelWidthBox->Text.c_str());
    int height = atoi(visModelHeightBox->Text.c_str());
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

    for (int i = 0; i < models.size(); i++)
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


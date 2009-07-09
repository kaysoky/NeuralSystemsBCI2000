//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "BCI2000CertificationGUI.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBCICertificationGUI *BCICertificationGUI;
//---------------------------------------------------------------------------
__fastcall TBCICertificationGUI::TBCICertificationGUI(TComponent* Owner)
	: TForm(Owner)
{
	mCurTask = -1;
	runThread = NULL;
	mCurIni = "";
	if (!init(mCurIni))
	{
		exit(-1);
	}
}
//---------------------------------------------------------------------------

bool TBCICertificationGUI::init(string iniFile)
{
    if (!parseCfg(&mThresh,&mResFile, &mDatDir, &mMinReqs))
	{
		ShowMessage("Unable to locate BCI2000Certification.cfg. Make sure this file is located in BCI2000/tools/BCI2000Certification before continuing.");
		return false;
	}
	if (iniFile != ""){
	if (!mCT.parseIni(iniFile))
	{
		if (mCT.taskReturnCode() == -1)
        {
			ShowMessage(string("Unable to find " + iniFile + ". Make sure this file is located in BCI2000/tools/BCI2000Certification before continuing.").c_str());
			return false;
        }
		else if (mCT.taskReturnCode() == -3)
        {
			ShowMessage("Duplicate task names found in BCI2000Certification.ini. Remove or rename duplicates, and try again.");
			return false;
		}
		/*
		stringstream st;
		st <<"Some configurations in the BCI2000Certification.ini file are incomplete. The following valid configurations will be run:"<<endl;
		for (int i=0; i < mCT.nTasks(); i++)
			if (!mCT[i].skip)
				st << "* " <<mCT[i].taskName<<endl;
		int ret = MessageDlg(st.str().c_str(), mtWarning, TMsgDlgButtons() << mbOK << mbCancel ,0);
		if (ret == mrCancel)
			return false;    */
	}
	}

	TListItem *tmpParmItem;
	taskList->Clear();
	for (int i = 0; i < mCT.nTasks(); i++)
	{
		tmpParmItem = taskList->Items->Add();
		tmpParmItem->Caption = mCT[i].taskName.c_str();
		tmpParmItem->Checked = !mCT[i].skip;
	}
	globalSigSrcBox->Text = mCT.GlobalSource().c_str();
	return true;
}

//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::delPrmBtnClick(TObject *Sender)
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	TListItem *tmpItem = parmsList->Selected;
	if (tmpItem == NULL) return;
	int pos = tmpItem->Index;
	mCT[mCurTask].delParm(pos);
	updateParmPanel();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::addPrmBtnClick(TObject *Sender)
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	OpenDialog1->DefaultExt = "prm";
	OpenDialog1->Filter = "BCI2000 Parm Files (*prm)|*.prm";
	OpenDialog1->Options.Clear();
	OpenDialog1->Options << ofFileMustExist << ofAllowMultiSelect;
	if (OpenDialog1->Execute())
	{
		for (int i = 0; i < OpenDialog1->Files->Count; i++)
		{
			mCT[mCurTask].addParm(string(OpenDialog1->Files[i].Text.c_str()));
		}
	}
	updateParmPanel();
}
//---------------------------------------------------------------------------
					 //OpenDialog1->Files->Strings[i]
void __fastcall TBCICertificationGUI::dataSaveBtnClick(TObject *Sender)
{
	/*AnsiString newDir;
	if (SelectDirectory(newDir, TSelectDirOpts() << sdAllowCreate << sdPerformCreate << sdPrompt, 0))
	{
		dataSaveBox->Text = newDir;
	} */
}
//---------------------------------------------------------------------------


void __fastcall TBCICertificationGUI::taskListClick(TObject *Sender)
{
	mCurTask = taskList->ItemIndex;
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	mCT[mCurTask].skip = !taskList->Items[0][mCurTask]->Checked;
    updateParmPanel();

}
//---------------------------------------------------------------------------
void TBCICertificationGUI::updateParmPanel()
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	taskNameBox->Text = mCT[mCurTask].taskName.c_str();
	parmsList->Clear();
	TListItem *tmp;
	for (int i=0; i < mCT[mCurTask].parmFileDisp.size(); i++)
	{
		tmp = parmsList->Items->Add();
		tmp->Caption = mCT[mCurTask].parmFileDisp[i].c_str();
    }
	if (mCT[mCurTask].amp.ch < 0)
		ampBox->Text = "";
	else
		ampBox->Text = mCT[mCurTask].amp.ch+1;

	if (mCT[mCurTask].dAmp.ch < 0)
		digAmpBox->Text = "";
	else
		digAmpBox->Text = mCT[mCurTask].dAmp.ch+1;

	if (mCT[mCurTask].vid.ch < 0)
		vidBox->Text = "";
	else
		vidBox->Text = mCT[mCurTask].vid.ch+1;
	vidStateBox->Text = mCT[mCurTask].vid.state.c_str();
	stringstream strTmp;
	for (int k = 0; k < mCT[mCurTask].vid.stateVal.size(); k++)
		 strTmp << mCT[mCurTask].vid.stateVal[k] << " ";

	vidStateValuesBox->Text = strTmp.str().c_str();
	if (mCT[mCurTask].aud.ch < 0)
		audioBox->Text = "";
	else
		audioBox->Text = mCT[mCurTask].aud.ch+1;
	audioStateBox->Text = mCT[mCurTask].aud.state.c_str();
	strTmp.str("");
	for (int k = 0; k < mCT[mCurTask].aud.stateVal.size(); k++)
		strTmp << mCT[mCurTask].aud.stateVal[k] << " ";

	audioStateValuesBox->Text = strTmp.str().c_str();
	sampleRateBox->Text = mCT[mCurTask].sampleRate;
	SBSbox->Text = mCT[mCurTask].blockSize;

	sigSourceBox->Text = mCT[mCurTask].SignalSource.c_str();
	sigProcBox->Text = mCT[mCurTask].SigProc.c_str();
	appBox->Text = mCT[mCurTask].App.c_str();


}

void TBCICertificationGUI::updateParm()
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	//check for spaces
	if (string(taskNameBox->Text.c_str()).find_first_of(" ") != string::npos)
	{
		ShowMessage("The task name may not contain spaces.");
		return;
	}
	mCT[mCurTask].taskName = taskNameBox->Text.c_str();
	taskList->Items[0][mCurTask]->Caption =taskNameBox->Text;

	try{
		mCT[mCurTask].amp.ch = atoi(ampBox->Text.c_str())-1;
	}
	catch(...){
        //mCT[mCurTask].amp.ch = -1;
	}

	try{
		mCT[mCurTask].dAmp.ch = atoi(digAmpBox->Text.c_str())-1;
	}
	catch(...){
    } 

	try{
		mCT[mCurTask].vid.ch = atoi(vidBox->Text.c_str())-1;
	}
	catch(...){
	}

	if (string(vidStateBox->Text.c_str()).find_first_of(" ") == string::npos)
	{
		mCT[mCurTask].vid.state = vidStateBox->Text.c_str();
	}


	try{
		stringstream ss;
		ss.str(vidStateValuesBox->Text.c_str());
		int tmpV, pos = 0;
		while (!ss.eof()){
			ss >> tmpV;
			mCT[mCurTask].vid.stateVal[pos++] = tmpV;
		}
	}
	catch(...){
	}


	try{
		mCT[mCurTask].aud.ch = atoi(audioBox->Text.c_str())-1;
	}
	catch(...){
	}

	if (string(audioStateBox->Text.c_str()).find_first_of(" ") == string::npos)
	{
		mCT[mCurTask].aud.state = audioStateBox->Text.c_str();
	}


	try{
		//mCT[mCurTask].aud.stateVal = atoi(audioStateValuesBox->Text.c_str());
		stringstream ss;
		ss.str(audioStateValuesBox->Text.c_str());
		int tmpV;
		int pos = 0;
		while (!ss.eof()){
			ss >>tmpV;
			mCT[mCurTask].aud.stateVal[pos++] = tmpV;
		}
	}
	catch(...){
	}

	mCT[mCurTask].SignalSource = sigSourceBox->Text.c_str();
	mCT[mCurTask].SigProc = sigProcBox->Text.c_str();
	mCT[mCurTask].App = appBox->Text.c_str();

	mCT[mCurTask].sampleRate = atof(sampleRateBox->Text.c_str());
	mCT[mCurTask].blockSize = atof(SBSbox->Text.c_str());

	updateSkips();
	updateParmPanel();
}

void __fastcall TBCICertificationGUI::parmsListInfoTip(TObject *Sender,
      TListItem *Item, AnsiString &InfoTip)
{
	InfoTip = Item->Caption;	
}
//---------------------------------------------------------------------------

void TBCICertificationGUI::updateSkips()
{
	for (size_t i = 0; i < taskList->Items[0].Count; i++)
		mCT[i].skip = !taskList->Items[0][i]->Checked;
}

void __fastcall TBCICertificationGUI::taskNameBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::sigSourceBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::sigProcBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::appBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::ampBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::digAmpBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::vidBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::vidStateBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::vidStateValuesBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::audioBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::audioStateBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::audioStateValuesBoxExit(TObject *Sender)
{
	updateParm();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::getSigSrcBtnClick(TObject *Sender)
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	OpenDialog1->DefaultExt = "exe";
    OpenDialog1->Filter = "BCI2000 Exe File (*.exe)|*.exe";
	OpenDialog1->Options.Clear();
	OpenDialog1->Options << ofFileMustExist;
	if (OpenDialog1->Execute())
	{
		string tmp(OpenDialog1->Files[0].GetText());
		int pos = tmp.find_last_of("\\");
		if (pos != string::npos)
			mCT[mCurTask].SignalSource = tmp.substr(pos+1);
		else
			mCT[mCurTask].SignalSource = tmp;
	}
	updateParmPanel();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::getSigProcBtnClick(TObject *Sender)
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	OpenDialog1->DefaultExt = "exe";
	OpenDialog1->Filter = "BCI2000 Exe File (*.exe)|*.exe";
	OpenDialog1->Options.Clear();
	OpenDialog1->Options << ofFileMustExist;
	if (OpenDialog1->Execute())
	{
		string tmp(OpenDialog1->Files[0].GetText());
		int pos = tmp.find_last_of("\\");
		if (pos != string::npos)
			mCT[mCurTask].SigProc = tmp.substr(pos+1);
		else
			mCT[mCurTask].SigProc = tmp;
	}
	updateParmPanel();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::getAppBtnClick(TObject *Sender)
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	OpenDialog1->DefaultExt = "exe";
	OpenDialog1->Filter = "BCI2000 Exe File (*.exe)|*.exe";
	OpenDialog1->Options.Clear();
	OpenDialog1->Options << ofFileMustExist;
	if (OpenDialog1->Execute())
	{
		string tmp(OpenDialog1->Files[0].GetText());
		int pos = tmp.find_last_of("\\");
		if (pos != string::npos)
			mCT[mCurTask].App = tmp.substr(pos+1);
		else
			mCT[mCurTask].App = tmp;
	}
	updateParmPanel();
}
//---------------------------------------------------------------------------




void __fastcall TBCICertificationGUI::winLeftBoxExit(TObject *Sender)
{
	updateGlobal();

}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::winTopBoxExit(TObject *Sender)
{
	updateGlobal();

}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::winWidthBoxExit(TObject *Sender)
{
	updateGlobal();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::winHeightBoxExit(TObject *Sender)
{
	updateGlobal();

}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::dataSaveBoxExit(TObject *Sender)
{
	mCT.mDataDir = dataSaveBox->Text.c_str();	
}
//---------------------------------------------------------------------------
					  
__fastcall TBCICertificationGUI::RunThread::RunThread(bool s, TBCICertificationGUI* pr)
	: TThread(s)
{
	p = pr;
	threadRunning = false;
}

void __fastcall TBCICertificationGUI::RunThread::Execute()
{
	p->mCT.reset();
	p->mCT.nextTask();
	p->infoBox->Clear();
	while (p->mCT.tasksRemain() && !Terminated)
	{
		Sleep(2000);
		p->infoBox->Lines->Add(string("Launching " + p->mCT[p->mCT.GetCurrentTask()].taskName).c_str());
		p->mCT.launchProgs();
		p->mCT.monitorProgs();
		p->mCT.nextTask();
	}
}


void TBCICertificationGUI::updateGlobal()
{
	mCT.mDataDir = dataSaveBox->Text.c_str();
	try{
		mCT.mWinWidth = atoi(winWidthBox->Text.c_str());
		mCT.useWinWidth = true;
	}
	catch(...){
		mCT.useWinWidth = false;
	}
	try{
		mCT.mWinLeft = atoi(winLeftBox->Text.c_str());
		mCT.useWinLeft = true;
	}
	catch(...){
	mCT.useWinLeft = false;
	}
	try{
		mCT.mWinHeight = atoi(winHeightBox->Text.c_str());
		mCT.useWinHeight = true;
	}
	catch(...){
	mCT.useWinHeight = false;
	}
	try{
		mCT.mWinTop = atoi(winTopBox->Text.c_str());
		mCT.useWinTop = true;
	}
	catch(...){
	mCT.useWinTop = false;
	}
	string tmp(globalSigSrcBox->Text.c_str());
	int pos = tmp.find_last_of("\\");
	if (pos == string::npos)
	{
		mCT.setGlobalSource(tmp.substr(pos+1));
	}
	else
		mCT.setGlobalSource(tmp);

	for (int i = 0; i < taskList->Items[0].Count; i++)
		mCT[i].skip = !taskList->Items[0][i]->Checked;
}
void __fastcall TBCICertificationGUI::startBtnClick(TObject *Sender)
{
	updateGlobal();
	if (runThread != NULL)
		if (runThread->threadRunning)
		{
			ShowMessage("BCI2000 is already running tests. Press cancel and close BCI2000 to start again");
			return;
		}
		
	runThread = new RunThread(true, this);
	runThread->FreeOnTerminate = false;
	runThread->Resume();
}

bool TBCICertificationGUI::checkRemoveData()
{
	//get files to delete
	vector<string> fNames;
	parseDir(mCT.mDataDir, &fNames);
	if (fNames.size() > 0)
    {
		int ret = MessageDlg("Data exists in the specified output directory. Do you want to remove this data before continuing? If you answer no, this data will be included in the analysis",
							   mtInformation, TMsgDlgButtons() << mbYes << mbNo << mbAbort, 0);

		if (ret == mrAbort)
			return false;

		if (mrYes){
			for (int i = 0; i < fNames.size(); i++)
				remove(fNames[i].c_str());
		}
    }
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::FormClose(TObject *Sender,
      TCloseAction &Action)
{
	delete runThread;	
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::cancelBtnClick(TObject *Sender)
{
	if (runThread == NULL)
		return;
	if (!runThread->Suspended && !runThread->Terminated)
	{
		runThread->Terminate();
		infoBox->Lines->Add("Waiting for execution to complete...");
		int ret = runThread->WaitFor(); //wait a minute
		//if (ret != TWaitResult::wrSignaled)
		  //	ShowMessage("There was a timeout or error waiting for the test to complete.");
		delete runThread;
		runThread = NULL;
		return;
	}	
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::globalSigSrcBoxExit(TObject *Sender)
{
	updateGlobal();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::addTaskBtnClick(TObject *Sender)
{
	TaskType newTask;
	TListItem *tmpParmItem;
	newTask.taskName = "newTask";
	mCT.tasks.push_back(newTask);
    tmpParmItem = taskList->Items->Add();
	tmpParmItem->Caption = mCT[mCT.tasks.size()-1].taskName.c_str();
	tmpParmItem->Checked = false;
}



//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::delTaskBtnClick(TObject *Sender)
{
	TListItem *tmpItem = taskList->Selected;
	if (tmpItem == NULL) return;
	int pos = tmpItem->Index;
	taskList->DeleteSelected();
	if (pos < 0 || pos >= mCT.tasks.size())
		return;
		
	mCT.tasks.erase(mCT.tasks.begin()+pos);	
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::analyzeBtnClick(TObject *Sender)
{
	infoBox->Lines->Add("Starting analysis...");
	stringstream comm;
	comm << "start BCI2000CertAnalysis.exe -d " << dataSaveBox->Text.c_str() << " -i " << mCurIni.c_str() <<endl;
	system(comm.str().c_str());
}
//---------------------------------------------------------------------------


void __fastcall TBCICertificationGUI::Openini1Click(TObject *Sender)
{
	OpenDialog1->DefaultExt = "ini";
	OpenDialog1->Filter = "BCI2000 Cert ini Files (*.ini)|*.ini";
	OpenDialog1->Options.Clear();
	OpenDialog1->Options << ofFileMustExist << ofAllowMultiSelect;
	if (OpenDialog1->Execute())
	{
		mCurIni=(OpenDialog1->FileName.c_str());
		init(mCurIni);
		updateParmPanel();
	}

}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::Saveini1Click(TObject *Sender)
{
	updateParm();
	updateSkips();
	SaveDialog1->DefaultExt = "ini";
	SaveDialog1->FileName = mCurIni.c_str();
	SaveDialog1->Filter = string("BCI2000 Cert Ini File (*.ini)|*.ini").c_str();
	SaveDialog1->Options.Clear();
	SaveDialog1->Options << ofOverwritePrompt << ofNoChangeDir;
	if (SaveDialog1->Execute()){
                mCT.tasks.writeIni(SaveDialog1->FileName.c_str());
                mCurIni = SaveDialog1->FileName.c_str();
	}
		
}
//---------------------------------------------------------------------------


void __fastcall TBCICertificationGUI::sampleRateBoxExit(TObject *Sender)
{
	updateParm();	
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::SBSboxExit(TObject *Sender)
{
	updateParm();	
}
//---------------------------------------------------------------------------




void __fastcall TBCICertificationGUI::copyBtnClick(TObject *Sender)
{
	TListItem *tmpItem = taskList->Selected;
	if (tmpItem == NULL) return;
	int pos = tmpItem->Index;
	if (pos < 0 || pos >= mCT.tasks.size())
		return;

	TaskType newTask = mCT[pos];
	TListItem *tmpParmItem;
	//newTask.taskName = "newTask";
	mCT.tasks.push_back(newTask);
	tmpParmItem = taskList->Items->Add();
	tmpParmItem->Caption = mCT[mCT.tasks.size()-1].taskName.c_str();
	tmpParmItem->Checked = false;
}
//---------------------------------------------------------------------------


void __fastcall TBCICertificationGUI::getGlobSigSrcBtnClick(TObject *Sender)
{
 //	if (mCurTask < 0 || mCurTask > mCT.nTasks())
   //		return;
	OpenDialog1->DefaultExt = "exe";
	OpenDialog1->Filter = "BCI2000 Executable (*.exe)|*.exe";
	OpenDialog1->Options.Clear();
	OpenDialog1->Options << ofFileMustExist;
	if (OpenDialog1->Execute())
	{
		globalSigSrcBox->Text = OpenDialog1->FileName;
		/*for (int i = 0; i < OpenDialog1->Files->Count; i++)
		{
			mCT[mCurTask].addParm(string(OpenDialog1->Files[i].Text.c_str()));
		} */
	}
	updateParmPanel();
}
//---------------------------------------------------------------------------

void __fastcall TBCICertificationGUI::selectAllCheckClick(TObject *Sender)
{
	bool st = selectAllCheck->Checked;
	for (int i = 0; i < taskList->Items[0].Count; i++){
		taskList->Items[0][i]->Checked = st;
		mCT[mCurTask].skip = !st;
	}	
}
//---------------------------------------------------------------------------


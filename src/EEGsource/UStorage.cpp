//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "UStorage.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Unit1::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   TDataStorage
// Purpose:    The constructor of the TDataStorage class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
__fastcall TDataStorage::TDataStorage(PARAMLIST *paramlist) : TThread(true)
{
int     i;
char    line[255];
char slash[2];

slash[0]= 0x5c;
slash[1]= 0;

 // data storage should have very low priority
 Priority=tpLowest;

 FName = new char [255];

 bcidtry=NULL;
 event=new TEvent(NULL, false, false, "");

 for (i=0; i<MAX_BUFFERS; i++)
  {
  critsec[i]=new TCriticalSection;
  buffer[i]=NULL;
  length[i]=0;
  }

  strcpy(line, "Storage string FileInitials= c:");
  strcat(line,slash);
  strcat(line,"data a z // Initials of file name (max. 8 characters)");

  paramlist->AddParameter2List(line, strlen(line));
  strcpy(line, "Storage string SubjectName= Name Name a z // subject alias (max. 8 characters)");
  paramlist->AddParameter2List(line, strlen(line));
  strcpy(line, "Storage string SubjectSession= 001 001 0 999 // session number (max. 3 characters)");
  paramlist->AddParameter2List(line, strlen(line));
  strcpy(line, "Storage string SubjectRun= 01 01 0 99 // digit run number (max. 3 characters)");
  paramlist->AddParameter2List(line, strlen(line));
  strcpy(line, "Storage string StorageTime= 16:15 Time a z // time of beginning of data storage");
  paramlist->AddParameter2List(line, strlen(line));
  strcpy(line, "Storage int AutoIncrementRunNo= 1 1 0 1 // 0: no auto increment 1: auto increment at Initialize)");
  paramlist->AddParameter2List(line, strlen(line));
  strcpy(line, "Storage int SavePrmFile= 0 1 0 1 // 0/1: don't save/save additional parameter file");
  paramlist->AddParameter2List(line, strlen(line));
  AlreadyIncremented = true;
  OldRunNo = 0;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   ~TDataStorage
// Purpose:    The destructor of the TDataStorage class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
__fastcall TDataStorage::~TDataStorage()
{
int     i;

 Terminate();
 event->SetEvent();
 WaitFor();

 delete [] FName;

 if (bcidtry) delete bcidtry;
 bcidtry=NULL;

 if (event) delete event;
 event=NULL;

 for (i=0; i<MAX_BUFFERS; i++)
  {
  if (critsec[i]) delete critsec[i];
  if (buffer[i]) delete [] buffer;
  critsec[i]=NULL;
  buffer[i]=NULL;
  length[i]=0;
  }
}
//---------------------------------------------------------------------------


void __fastcall TDataStorage::Execute()
{
int     i;
short   value;
FILE    *fp;

 while (!Terminated)
  {
  // critsec_event->Acquire();
  event->WaitFor(100);
  // critsec_event->Release();
  // go through all the buffers and store the ones that hold data
  for (i=0; i<MAX_BUFFERS; i++)
   {
   critsec[i]->Acquire();       // acquire a lock for this buffer
   // buffer present ?
   if (length[i] > 0)
      {
      fp=fopen(FName, "ab");
      fwrite(buffer[i], length[i], 1, fp);
      fclose(fp);
      delete [] buffer[i];
      buffer[i]=NULL;
      length[i]=0;
      AlreadyIncremented = false;
      }
   critsec[i]->Release();       // release the lock for this buffer
   }
  }

 ReturnValue=1;
}
//---------------------------------------------------------------------------


void TDataStorage::Initialize(PARAMLIST *Newparamlist, STATELIST *NewStateList, STATEVECTOR *Newstatevector)
{
static int NewRunNo=1;
FILE    *fp;
int     i;

 paramlist = Newparamlist;
 statelist = NewStateList;
 statevector = Newstatevector;

 max=0;

 paramlist = Newparamlist;
 statelist = NewStateList;
 statevector = Newstatevector;
 Channels = atoi(paramlist->GetParamPtr("SoftwareCh")->GetValue());
 StateVectorLen = statevector->GetStateVectorLength();

 // determine, whether we want to save a parameter file or not
 if (atoi(paramlist->GetParamPtr("SavePrmFile")->GetValue()) == 1)
    saveprmfile=true;
 else
    saveprmfile=false;

 // incrementing run no. or not

    NewRunNo = atoi(paramlist->GetParamPtr("SubjectRun")->GetValue());
    if( NewRunNo < OldRunNo ) NewRunNo= OldRunNo;
    if( ( NewRunNo == OldRunNo ) && ( AlreadyIncremented == false ) )
    {
        NewRunNo++;
        sprintf(FName, "%d", NewRunNo);
        paramlist->GetParamPtr("SubjectRun")->SetValue(FName);
        AlreadyIncremented = true;
    }

 CreateFileName();
 OldRunNo = NewRunNo;

 // determine whether we have to write the header
 // can't write a buffer at the same time
 for (i=0; i<MAX_BUFFERS; i++)
   critsec[i]->Acquire();
 if( (fp = fopen(FName, "ab") ) == NULL )
         Application->MessageBox("In Initialize: Could not open data file ", "Error", MB_OK);

 if (fp)
    {
    fseek(fp, 0L, SEEK_END);
    if (ftell(fp) < 10)
       {
       fclose(fp);
       WriteHeader();
       }
    fclose(fp);
    }
 // release the locks
 for (i=0; i<MAX_BUFFERS; i++)
   critsec[i]->Release();
}


void TDataStorage::CreateFileName()
{
 if (bcidtry) delete bcidtry;
 bcidtry= new BCIDtry();

        char path[160];
        char slash[2];
        slash[0]= 0x5c;
        slash[1]= 0;

        AnsiString FInit = AnsiString (paramlist->GetParamPtr("FileInitials")->GetValue());
        AnsiString SRun = AnsiString (atoi(paramlist->GetParamPtr("SubjectRun")->GetValue()));
        if (SRun.Length()==1) SRun = "0" + SRun;
        AnsiString SSes = AnsiString (atoi(paramlist->GetParamPtr("SubjectSession")->GetValue()));
        if (SSes.Length()==1) SSes = "00" + SSes;
        if (SSes.Length()==2) SSes = "0" + SSes;
        AnsiString SName = AnsiString(paramlist->GetParamPtr("SubjectName")->GetValue());

        AnsiString Path= FInit;
        bcidtry->SetDir( Path.c_str() );
        bcidtry->ProcPath();
        bcidtry->SetName( SName.c_str() );
        bcidtry->SetSession( SSes.c_str() );

        strcpy( FName, bcidtry->ProcSubDir() );
        strcat( FName,slash);

        AnsiString AName = SName + "S" + SSes + "R" + SRun + ".dat";

        strcat(FName, AName.c_str());

}


char *TDataStorage::GetFileName()
{
 return (FName);
}


// --------------------- writing the header at begin of session ------------------------
void TDataStorage::WriteHeader()
{
FILE    *fp;
char    *HeaderBuf;
char    Line[255];
char    *PLine;
fpos_t  pos;

 if( ( fp = fopen(FName,"w+b") ) == NULL )
        Application->MessageBox("In Write Header: Could not open data file ", "Error", MB_OK);

 if (fp)
    {
    fprintf(fp, "HeaderLen=       SourceCh= %d StatevectorLen= %d \r\n", Channels, StateVectorLen);
    fprintf(fp, "[ State Vector Definition ] \r\n");
    for (int i=0; i<statelist->GetNumStates(); i++)
     {
     fprintf(fp, statelist->GetStatePtr(i)->GetStateLine());
     fprintf(fp, "\r\n");
     }
    fprintf(fp, "[ Parameter Definition ] \r\n");
    for (int i=0; i<paramlist->GetNumParameters(); i++)
     {
     fprintf(fp, paramlist->GetParamPtr(i)->GetParamLine());
     fprintf(fp, "\r\n");
     }
    fprintf(fp, "\r\n");
    fgetpos(fp, &pos);
    fseek(fp, 0L, SEEK_SET);
    fprintf(fp, "HeaderLen= %ld", pos);
    fseek(fp, pos, SEEK_SET);
    fclose(fp);
    }

// --------- Saving parameters into PRM-File additionally -----------------------
if (saveprmfile)
   {
   strcpy(Line, FName);
   PLine = strstr(Line, ".dat\0");
   strcpy( PLine, ".prm");
   if (!paramlist->SaveParameterList(Line)) Application->MessageBox("Could not write parameter file !", NULL, MB_OK);
   }
// --------- end of PRM saving --------------------------------------------------
}



// ----------------- main storing procedure -------------------------------------
// --- stores the whole signal block to the disk --------------------------------
// --- the file is opened and closed with every call ----------------------------
bool TDataStorage::Write2Disk(GenericIntSignal *my_signal)
{
int     i, ptr, s, t;
char    *cur_ptr;

 // go through all the buffers and determine which one is free
 // start from the end to fill the buffer after the last one used
 ptr=-1;
 for (i=MAX_BUFFERS-1; i>=0; i--)
  {
  critsec[i]->Acquire();       // acquire a lock for this buffer
  if ((length[i] != 0) && (ptr == -1))
     ptr=i+1;
  critsec[i]->Release();       // release the lock for this buffer
  }

 // all buffers filled
 if (ptr >= MAX_BUFFERS-1)
    return(false);

 // assign appropriate buffer slot
 if (ptr == -1) // no buffer currently filled
    i=0;
 else
    i=ptr;

 if (i > max) max=i;

 critsec[i]->Acquire();       // acquire a lock for this buffer
 // determine the buffer size
 length[i]=my_signal->Channels*my_signal->MaxElements*sizeof(short)+StateVectorLen*my_signal->MaxElements;
 // length[i]=my_signal->Channels*my_signal->MaxElements*sizeof(short)+2*my_signal->MaxElements;
 buffer[i]=new char[length[i]];
 cur_ptr=buffer[i];
 // write the actual data into the buffer
 for (s=0; s<my_signal->MaxElements; s++)
  {
  for (t=0; t<my_signal->Channels; t++)
   {
   // write the samples into memory
   *((short *)cur_ptr)=my_signal->GetValue(t, s);
   cur_ptr+=sizeof(short);
   }
  // write the statevector into memory
  // *((short *)cur_ptr)=0;
  // cur_ptr+=sizeof(short);
  memcpy(cur_ptr, statevector->GetStateVectorPtr(), StateVectorLen);
  cur_ptr+=StateVectorLen;
  }
 critsec[i]->Release();       // release the lock for this buffer

 // notify the main loop that data is in buffer
 event->SetEvent();

 return(true);
}

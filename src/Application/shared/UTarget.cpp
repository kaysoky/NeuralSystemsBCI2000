//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UTarget.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   TARGETLIST
// Purpose:    This is the constructor for the TARGETLIST class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TARGETLIST::TARGETLIST()
{
 target_list=new TList;
 critsec=new TCriticalSection;

 // set the mode to no prediction
 predictionmode=MODE_NORMAL;
}



// **************************************************************************
// Function:   ~TARGETLIST
// Purpose:    This is the destructor for the TARGETLIST class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TARGETLIST::~TARGETLIST()
{
 DeleteTargets();

 if (target_list) delete target_list;
 if (critsec) delete critsec;

 target_list=NULL;
 critsec=NULL;
}


// **************************************************************************
// Function:   DeleteTargets
// Purpose:    This function clears the list of targets and frees the associated memory
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TARGETLIST::DeleteTargets()
{
int     i;
TARGET  *cur_target;

 if (!critsec || !target_list)
    return;

 critsec->Acquire();

  // Clean up – must free memory for the items as well as the list
  for (i=0; i<target_list->Count; i++)
   {
   cur_target=(TARGET *)target_list->Items[i];
   delete cur_target;
   }

  target_list->Clear();

 critsec->Release();
}


// **************************************************************************
// Function:   GetMaxTargetID
// Purpose:    This function returns the biggest targetID of all targets in this list
// Parameters: N/A
// Returns:    biggest target ID, if OK, or
//             TARGETID_NOID, if there is a problem
// **************************************************************************
int TARGETLIST::GetMaxTargetID()
{
int     i, maxID;
TARGET  *cur_target;

 if (!target_list)
    return(TARGETID_NOID);

 critsec->Acquire();

 maxID=-1;
 for (i=0; i<target_list->Count; i++)
  {
  cur_target=(TARGET *)target_list->Items[i];
  if (cur_target)
     if (cur_target->targetID > maxID)
        maxID=cur_target->targetID;
  }

 critsec->Release();

 return(maxID);
}


// **************************************************************************
// Function:   GetNumTargets
// Purpose:    This function returns the number of targets in this list
// Parameters: N/A
// Returns:    number of targets in the list
// **************************************************************************
int TARGETLIST::GetNumTargets()
{
 return(target_list->Count);
}


// **************************************************************************
// Function:   RenderAllTargets
// Purpose:    This function renders all targets onto the specified canvas
// Parameters: canvas - pointer to the canvas that will hold the targets
// Returns:    N/A
// **************************************************************************
void TARGETLIST::RenderTargets(TForm *form, TRect destrect)
{
int     i;
TARGET  *cur_target;

 critsec->Acquire();
 for (i=0; i<target_list->Count; i++)
  {
  cur_target=(TARGET *)target_list->Items[i];
  cur_target->RenderTarget(form, destrect);
  }
 critsec->Release();
}


// **************************************************************************
// Function:   HighlightTargets
// Purpose:    This function highlist all targets in the list
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TARGETLIST::HighlightTargets()
{
int     i;
TARGET  *cur_target;

 critsec->Acquire();
 for (i=0; i<target_list->Count; i++)
  {
  cur_target=(TARGET *)target_list->Items[i];
  cur_target->HighlightTarget();
  }
 critsec->Release();
}


// **************************************************************************
// Function:   GetCurrentBackupPosition
// Purpose:    returns the screen position of the current backup
// Parameters: N/A
// Returns:    -1, if not found
//             screen position otherwise
// **************************************************************************
int TARGETLIST::GetCurrentBackupPosition()
{
int     position;
int     i;
TARGET  *cur_target;

 position=-1;
 for (i=0; i<target_list->Count; i++)
  {
  cur_target=(TARGET *)target_list->Items[i];
  if (cur_target->targetID == TARGETID_BACKUP)
     position=cur_target->targetposition;
  }

 return(position);
}



// **************************************************************************
// Function:   TargetsSelected
// Purpose:    This function detects whether there is collision with ANY target
// Parameters: cursor - pointer to the cursor
// Returns:    pointer to the target, if it has been selected
//             NULL, if no target has been selected
// **************************************************************************
TARGET *TARGETLIST::TargetsSelected(CURSOR *cursor)
{
int     i;
TARGET  *cur_target, *selected;

 critsec->Acquire();
 selected=NULL;
 for (i=0; i<target_list->Count; i++)
  {
  cur_target=(TARGET *)target_list->Items[i];
  selected=cur_target->TargetSelected(cursor);
  if (selected) break;
  }
 critsec->Release();

 return(selected);
}


// **************************************************************************
// Function:   HideTargets
// Purpose:    This function hides all targets
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TARGETLIST::HideTargets()
{
int     i;
TARGET  *cur_target;

 critsec->Acquire();

 for (i=0; i<target_list->Count; i++)
   {
   cur_target=(TARGET *)target_list->Items[i];
   cur_target->HideTarget();
   }

 critsec->Release();
}


// **************************************************************************
// Function:   GetTargetPtr
// Purpose:    This function returns the pointer of a target specified by its caption
//             Comparisons are case-insensitive
// Parameters: char *caption - caption of the sought after target
// Returns:    pointer of target - on success
//             NULL              - on failure (i.e., target not found)
// **************************************************************************
TARGET *TARGETLIST::GetTargetPtr(AnsiString caption)
{
AnsiString      cur_caption;
TARGET  *ptr, *cur_ptr;
int     i;

 critsec->Acquire();

 ptr=NULL;
 for (i=0; i<target_list->Count; i++)
  {
  cur_ptr=(TARGET *)target_list->Items[i];
  if (cur_ptr)
     if (cur_ptr->Caption.AnsiCompareIC(caption) == 0)
        {
        ptr=cur_ptr;
        break;
        }
  }

 critsec->Release();
 return(ptr);
}



// **************************************************************************
// Function:   GetTargetPtr
// Purpose:    This function returns the pointer of a target specified by its ID
// Parameters: my_targetID - ID of sought after target
// Returns:    pointer of target - on success
//             NULL              - on failure (i.e., target not found)
// **************************************************************************
TARGET *TARGETLIST::GetTargetPtr(int my_targetID)
{
TARGET  *ptr, *cur_ptr;
int     i;

 critsec->Acquire();

 ptr=NULL;
 for (i=0; i<target_list->Count; i++)
  {
  cur_ptr=(TARGET *)target_list->Items[i];
  if (cur_ptr->targetID == my_targetID)
     {
     ptr=cur_ptr;
     break;
     }
  }

 critsec->Release();
 return(ptr);
}



// **************************************************************************
// Function:   GetTargetPtr
// Purpose:    This function returns the pointer of a target specified by its ID and its display position
//             this is an important function if there are more targets with the same ID (e.g., more than one blank target)
// Parameters: my_targetID - ID of sought after target
//             displaypos  - displayposition of sought after target
// Returns:    pointer of target - on success
//             NULL              - on failure (i.e., target not found)
// **************************************************************************
TARGET *TARGETLIST::GetTargetPtr(int my_targetID, int displaypos)
{
TARGET  *ptr, *cur_ptr;
int     i;

 critsec->Acquire();

 ptr=NULL;
 for (i=0; i<target_list->Count; i++)
  {
  cur_ptr=(TARGET *)target_list->Items[i];
  if ((cur_ptr->targetID == my_targetID) && (cur_ptr->targetposition == displaypos))
     {
     ptr=cur_ptr;
     break;
     }
  }

 critsec->Release();
 return(ptr);
}



// **************************************************************************
// Function:   GetTargetID
// Purpose:    This function returns the ID of the target specified by its display position
// Parameters: targetnum - position on screen of sought after target
// Returns:    targetID ... success
//             TARGETID_NOID - on failure (i.e., target not found)
// **************************************************************************
int TARGETLIST::GetTargetID(int targetnum)
{
TARGET  *cur_ptr;
int     i, targetID;

 critsec->Acquire();

 targetID=TARGETID_NOID;
 for (i=0; i<GetNumTargets(); i++)
  {
  cur_ptr=(TARGET *)target_list->Items[i];
  if (cur_ptr)
     if ((int)cur_ptr->targetposition == targetnum)
        targetID=cur_ptr->targetID;
  }

 critsec->Release();
 return(targetID);
}


// **************************************************************************
// Function:   GetTargetID
// Purpose:    This function returns the ID of the target specified by its caption
//             and by the target type (e.g., TARGETTYPE_CHARACTER)
//             Comparisons are case-insensitive
// Parameters: my_targetID - ID of sought after target
//             targettype  - target type of sought after target (see UTarget.h for definitions)
// Returns:    targetID ... success
//             TARGETID_NOID - on failure (i.e., target not found)
// **************************************************************************
int TARGETLIST::GetTargetID(AnsiString caption, int targettype)
{
TARGET  *cur_ptr;
int     i, targetID;

 critsec->Acquire();

 targetID=TARGETID_NOID;
 for (i=0; i<GetNumTargets(); i++)
  {
  cur_ptr=(TARGET *)target_list->Items[i];
  if (cur_ptr)
     if ((cur_ptr->Caption.AnsiCompareIC(caption) == 0) && (cur_ptr->targettype == targettype))
        targetID=cur_ptr->targetID;
  }

 critsec->Release();
 return(targetID);
}


// **************************************************************************
// Function:   Add
// Purpose:    This function adds a new target to the list
// Parameters: new_target - ID of new target
// Returns:    true  - on success
//             false - on failure (e.g., target already exists)
// **************************************************************************
bool TARGETLIST::Add(TARGET *new_target)
{
 /* we could have two identical targets (e.g., two blank targets at different positions)
 // does target already exist -> don't add
 if (GetTargetPtr(new_target->targetID) != NULL)
    return(false);*/

 // add target to the list of targets
 critsec->Acquire();
 target_list->Add(new_target);
 critsec->Release();
 return(true);
}



// **************************************************************************
// Function:   TARGET
// Purpose:    This is the constructor of the TARGET class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TARGET::TARGET(int my_targetID)
{
 targetID=my_targetID;

 Color=clYellow;
 Pen=new TPen();
 Pen->Style = psSolid;
 Pen->Color = clBlack;
 Pen->Width = 1;

 shape=NULL;
 icon=NULL;
 caption=NULL;

 IconFile="";
 Caption="";
}


// **************************************************************************
// Function:   ~TARGET
// Purpose:    This is the destructor of the TARGET class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TARGET::~TARGET()
{
 if (Pen) delete Pen;
 if (shape) delete shape;
 if (icon) delete icon;
 if (caption) delete caption;
 caption=NULL;
 Pen=NULL;
 shape=NULL;
 icon=NULL;
}


// **************************************************************************
// Function:   CloneTarget
// Purpose:    This function clones a particular target by "deep-copying" the old one
// Parameters: N/A
// Returns:    pointer to the cloned target
// **************************************************************************
TARGET *TARGET::CloneTarget()
{
TARGET *new_target;

 new_target=new TARGET(targetID);
 new_target->Caption=Caption;
 new_target->IconFile=IconFile;
 new_target->Color=Color;
 new_target->TextColor=TextColor;
 new_target->targetposition=targetposition;
 new_target->parentID=parentID;
 new_target->Top=Top;
 new_target->Left=Left;
 new_target->Width=Width;
 new_target->Height=Height;

 return(new_target);
}


// **************************************************************************
// Function:   TargetSelected
// Purpose:    This function detects whether there is collision with this particular target
// Parameters: cursor - pointer to the cursor
// Returns:    pointer to the target, if it has been selected
//             NULL, if it hasn't been selected
// **************************************************************************
TARGET *TARGET::TargetSelected(CURSOR *cursor)
{
TARGET  *selected;
int     cursorposx, cursorposy;         // in coordinates 0..65535

 // determine the cursor's "hot spot"
 cursorposx=cursor->Left+cursor->Width/2;
 cursorposy=cursor->Top+cursor->Height/2;

 // has the cursor hit the target ?
 // in this case, selection is purely based on coordinates; could also be based on dwell time
 selected=NULL;
 if ((cursorposx > Left) && (cursorposx <= Left+Width))
    if ((cursorposy > Top) && (cursorposy <= Top+Height))
       selected=this;

 return(selected);
}


// **************************************************************************
// Function:   RenderTarget
// Purpose:    This function renders this target onto the specified form
//             actually, it updates this target's rectangle's, icon's, and caption's properties
// Parameters: form     - pointer to the form that will hold the target
//             destrect - part of the form the status bar will be rendered into
// Returns:    N/A
// **************************************************************************
void TARGET::RenderTarget(TForm *form, TRect destrect)
{
int     destwidth, destheight;
int     scaledtop, scaledleft, scaledbottom, scaledright;
int     scaledtextsize, scaledtextposx, scaledtextposy;
float   scalex, scaley;

 destwidth=destrect.Width();
 destheight=destrect.Height();
 scalex=(float)destwidth/(float)65536;
 scaley=(float)destheight/(float)65536;

 scaledtop=(int)((float)Top*scaley+(float)destrect.Top);
 scaledbottom=(int)((float)(Top+Height)*scaley+(float)destrect.Top);
 scaledleft=(int)((float)Left*scalex+(float)destrect.Left);
 scaledright=(int)((float)(Left+Width)*scalex+(float)destrect.Left);

 scaledtextsize=(int)((float)TextHeight*scaley);
 form->Canvas->Font->Height=-scaledtextsize;
 form->Canvas->Font->Name="Arial";
 form->Canvas->Font->Style=TFontStyles() << fsBold;

 // create the rectangle, if it not already exists
 if (!shape)
    {
    shape=new TShape(Application);
    shape->Parent=form;
    }

 // set the rectangle's properties
 if (shape)
    {
    shape->Shape=stRectangle;
    shape->Brush->Color=Color;
    shape->Left=scaledleft;
    shape->Top=scaledtop;
    shape->Width=scaledright-scaledleft;
    shape->Height=scaledbottom-scaledtop;
    shape->Visible=false;
    shape->Enabled=true;
    }

 // create the icon, if not already exists
 if ((IconFile != "") && (!icon))
    {
    icon=new TImage(Application);
    icon->Parent=form;
    }

 // set the icon's properties
 if (icon)
    {
    icon->Visible=true;
    icon->Enabled=true;
    icon->Stretch=true;
    icon->Left=scaledleft+1;
    icon->Top=scaledtop+1;
    icon->Width=scaledright-scaledleft-2;
    icon->Height=scaledbottom-scaledtop-2;
    try {icon->Picture->LoadFromFile(IconFile);}
    catch(...)
         {
         delete icon;
         icon=NULL;
         }
    }

 // write the text, if any
 if ((Caption != "") && (!caption))
    {
    caption=new TLabel(Application);
    caption->Parent=form;
    }

 // set the text's properties
 if (caption)
    {
    caption->Visible=true;
    caption->Enabled=true;
    caption->Caption=Caption;
    caption->Layout=tlBottom;
    caption->Transparent=true;
    caption->Font->Color=TextColor;
    caption->Font->Height=-scaledtextsize;
    caption->Font->Name="Arial";
    caption->Font->Style=TFontStyles() << fsBold;
    scaledtextposx=abs((scaledleft+scaledright)/2-form->Canvas->TextWidth(Caption)/2);
    scaledtextposy=(scaledbottom+scaledtop)/2-caption->Height/2;
    caption->Left=scaledtextposx;
    caption->Top=scaledtextposy;
    }
}


// **************************************************************************
// Function:   HideTarget
// Purpose:    This function hides this target
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TARGET::HideTarget()
{
 // hide the rectangle, if it exists
 if (shape) shape->Visible=false;
 // hide the icon, if it exists
 if (icon) icon->Visible=false;
 // hide the text, if it exists
 if (caption) caption->Visible=false;
}



// **************************************************************************
// Function:   SetTextColor
// Purpose:    This function sets the text color of this target
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TARGET::SetTextColor(TColor new_color)
{
 // set the text color, if the text exists
 if (caption) caption->Font->Color=new_color;
}



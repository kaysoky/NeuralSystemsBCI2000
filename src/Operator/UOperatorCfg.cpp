#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <Registry.hpp>
#include <stdio.h>
#include <string>
#include <sstream>
#include <assert>

#include "..\shared\defines.h"
#include "UShowParameters.h"
#include "UEditMatrix.h"
#include "UOperatorCfg.h"
#include "UBCIError.h"
#include "UOperatorUtils.h"

using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfConfig *fConfig;
//---------------------------------------------------------------------------

__fastcall TfConfig::TfConfig(TComponent* Owner) : TForm(Owner)
{
  OperatorUtils::RestoreControl( this );
int     i, t;

 for(i=0; i<MAX_PARAMPERSECTION; i++)
  {
  ParamLabel[i]=NULL;
  ParamComment[i]=NULL;
  ParamValue[i]=NULL;
  ParamUserLevel[i]=NULL;
  for (t=0; t<3; t++)
   ParamButton[t][i]=NULL;
  }
}

__fastcall TfConfig::~TfConfig()
{
  OperatorUtils::SaveControl( this );
}
//---------------------------------------------------------------------------


int TfConfig::Initialize(PARAMLIST *my_paramlist, PREFERENCES *cur_preferences)
{
int             i, num_param;
PARAM           *cur_param;
AnsiString      tabname;

 paramlist=my_paramlist;
 num_param=paramlist->GetNumParameters();

 preferences=cur_preferences;

 DeleteAllTabs();

 // first, set archive to false
 for (i=0; i<num_param; i++)
  {
  cur_param=paramlist->GetParamPtr(i);
  if (cur_param)
     cur_param->archive=false;
  }

 // go through all the parameters and find out about the section names
 // set the Tab captions to the section names
 // use archive to flag, whether we already processed a parameter's section
 tabname="x";
 while (tabname != " ")
  {
  tabname=" ";
  for (i=0; i<num_param; i++)
   {
   cur_param=paramlist->GetParamPtr(i);
   if (cur_param)
      if (cur_param->valid)
         {
         // parameter has not been 'touched' yet and it's user level is smaller than the operator's user level
         if ((cur_param->archive == false) &&  (GetUserLevel(cur_param) <= preferences->UserLevel))
            {
            // a 'new' Section name ?
            if (tabname == " ")
               {
               tabname=cur_param->GetSection();
               cur_param->archive=true;
               }
            else
               {
               // 'touch' all parameters with the same section name
               if (tabname == cur_param->GetSection())
                  cur_param->archive=true;
               }
            }
         }
   }
  // create a new tab with the section name
  if (tabname != " ")
     CfgTabControl->Tabs->Insert(0, tabname);
  }

 if (CfgTabControl->Tabs->Count == 0)
    {
    CfgTabControl->Tabs->Insert(0, "No parameter visible");
    Application->MessageBox("No parameter visible ! Increase user level", "Message", MB_OK);
    }

 RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
 return(0);
}


void TfConfig::DeleteAllTabs()
{
int     i, num_tabs;

 // delete old Tabs, if present
 num_tabs=CfgTabControl->Tabs->Count;
 for (i=0; i<num_tabs; i++)
  CfgTabControl->Tabs->Delete(0);
}


void TfConfig::DeleteAllParameters()
{
int     i, t;

 for(i=0; i<MAX_PARAMPERSECTION; i++)
  {
  if (ParamLabel[i])
     delete(ParamLabel[i]);
  if (ParamComment[i])
     delete(ParamComment[i]);
  if (ParamValue[i])
     delete(ParamValue[i]);
  if (ParamUserLevel[i])
     delete(ParamUserLevel[i]);
  for (t=0; t<3; t++)
   {
   if (ParamButton[t][i])
      {
      delete(ParamButton[t][i]);
      ParamButton[t][i]=NULL;
      }
   }
  ParamLabel[i]=NULL;
  ParamComment[i]=NULL;
  ParamValue[i]=NULL;
  ParamUserLevel[i]=NULL;
#ifdef TRY_PARAM_INTERPRETATION
  delete ParamComboBox[ i ];
  ParamComboBox[ i ] = NULL;
  delete ParamCheckBox[ i ];
  ParamCheckBox[ i ] = NULL;
#endif // TRY_PARAM_INTERPRETATION
  }
}


// retrieves the user level of one particular parameter
int TfConfig::GetUserLevel(PARAM *param)
{
TRegistry       *my_registry;
AnsiString      keyname;
int             ret;
TStringList     *value_names;

 my_registry=new TRegistry();
 value_names=new TStringList();

 keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_PARAMETERS)+"\\"+AnsiString(param->GetName());
 ret=USERLEVEL_ADVANCED;
 if (my_registry->OpenKey(keyname, false))
    {
    my_registry->GetValueNames(value_names);
    // let's check whether the value "UserLevel" actually exists
    // (so that we don't throw many exceptions)
    if (value_names->IndexOf("UserLevel") > -1)
       {
       try  // if it gets here, it should actually exist
        {
        ret=my_registry->ReadInteger("UserLevel");
        }
       catch(...) {;}
       }
    }

 delete my_registry;
 delete value_names;
 return(ret);
}



// sets the user level of one particular parameter
void TfConfig::SetUserLevel(PARAM *param, int userlevel)
{
TRegistry       *my_registry;
AnsiString      keyname;

 my_registry=new TRegistry();

 try
  {
  keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_PARAMETERS)+"\\"+AnsiString(param->GetName());
  my_registry->CreateKey(keyname);
  }
 catch (...)
  {;}

 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     my_registry->WriteInteger("UserLevel", userlevel);
     }
    catch(...)
     {;}
    }

 delete my_registry;
 return;
}



// render all parameters in a particular section on the screen
int TfConfig::RenderParameters(AnsiString section)
{
int     count=0, i, num_param;
PARAM   *cur_param;
AnsiString valueline;

 num_param=paramlist->GetNumParameters();
 DeleteAllParameters();

 // go through all the parameters
 // if the parameter's section equals the current label of the tab,
 // then create the objects and draw them
 for (i=0; i<num_param; i++)
  {
  cur_param=paramlist->GetParamPtr(i);
  if (cur_param)
     {
     if ((section == cur_param->GetSection()) && (GetUserLevel(cur_param) <= preferences->UserLevel))
        {
        // render the parameter's name
        ParamLabel[count]=new TLabel(this);
        ParamLabel[count]->Left=LABELS_OFFSETX;
        ParamLabel[count]->Top=LABELS_OFFSETY+count*LABELS_SPACINGY;
        ParamLabel[count]->Caption=cur_param->GetName();
        ParamLabel[count]->Font->Style = TFontStyles()<< fsBold;
        ParamLabel[count]->Visible=true;
        ParamLabel[count]->Parent=CfgTabControl;
        ParamLabel[count]->Hint = cur_param->GetComment();
        ParamLabel[count]->ShowHint = true;
        // render the parameter's comment
        ParamComment[count]=new TLabel(this);
        ParamComment[count]->Left=COMMENT_OFFSETX;
        ParamComment[count]->Top=COMMENT_OFFSETY+count*COMMENT_SPACINGY;
        ParamComment[count]->Caption=cur_param->GetComment();
        ParamComment[count]->Hint = cur_param->GetComment();
        ParamComment[count]->ShowHint = true;
        ParamComment[count]->Font->Style = TFontStyles()<< fsItalic;
        ParamComment[count]->Visible=true;
        ParamComment[count]->Parent=CfgTabControl;
        // render the parameter's User Level track bar
        // ONLY, if the current user level is "advanced"
        if (preferences->UserLevel == USERLEVEL_ADVANCED)
           {
           ParamUserLevel[count]=new TTrackBar(this);
           ParamUserLevel[count]->Left=USERLEVEL_OFFSETX;
           ParamUserLevel[count]->Top=USERLEVEL_OFFSETY+count*USERLEVEL_SPACINGY;
           ParamUserLevel[count]->Width=USERLEVEL_WIDTHX;
           ParamUserLevel[count]->Min=1;
           ParamUserLevel[count]->Max=3;
           ParamUserLevel[count]->Position=GetUserLevel(cur_param);
           ParamUserLevel[count]->PageSize=1;
           ParamUserLevel[count]->OnChange=OnUserLevelChange;
           ParamUserLevel[count]->Visible=true;
           ParamUserLevel[count]->Parent=CfgTabControl;
           }
        // render the parameter's edit field, depending on the parameter data type
        ParamValue[count]=NULL;
        ParamButton[0][count]=NULL;
        ParamButton[1][count]=NULL;
        ParamButton[2][count]=NULL;
        if (strcmpi(cur_param->GetType(), "matrix") == 0)
           {
           ParamButton[0][count]=new TButton(this);
           ParamButton[0][count]->Caption="Edit Matrix";
           ParamButton[0][count]->Left=VALUE_OFFSETX;
           ParamButton[0][count]->Top=VALUE_OFFSETY+count*VALUE_SPACINGY;
           ParamButton[0][count]->Width=70;
           ParamButton[0][count]->Height=21;
           ParamButton[0][count]->OnClick=bEditMatrixClick;
           ParamButton[0][count]->Visible=true;
           ParamButton[0][count]->Parent=CfgTabControl;
           ParamButton[1][count]=new TButton(this);
           ParamButton[1][count]->Caption="Load Matrix";
           ParamButton[1][count]->Left=VALUE_OFFSETX+70+10;
           ParamButton[1][count]->Top=VALUE_OFFSETY+count*VALUE_SPACINGY;
           ParamButton[1][count]->Width=70;
           ParamButton[1][count]->Height=21;
           ParamButton[1][count]->OnClick=bLoadMatrixClick;
           ParamButton[1][count]->Visible=true;
           ParamButton[1][count]->Parent=CfgTabControl;
           ParamButton[2][count]=new TButton(this);
           ParamButton[2][count]->Caption="Save Matrix";
           ParamButton[2][count]->Left=VALUE_OFFSETX+70+10+70+10;
           ParamButton[2][count]->Top=VALUE_OFFSETY+count*VALUE_SPACINGY;
           ParamButton[2][count]->Width=70;
           ParamButton[2][count]->Enabled=false;
           ParamButton[2][count]->Height=21;
           // ParamButton[1][count]->OnClick=bEditMatrixClick;
           ParamButton[2][count]->Visible=true;
           ParamButton[2][count]->Parent=CfgTabControl;
           }
        else if( cur_param->GetNumValues() > 1 )
           {
           ParamValue[count]=new TEdit(this);
           ParamValue[count]->Left=VALUE_OFFSETX;
           ParamValue[count]->Top=VALUE_OFFSETY+count*VALUE_SPACINGY;
           ostringstream oss;
           for (size_t t=0; t<cur_param->GetNumValues(); t++)
            {
            oss << PARAM::encodedString( cur_param->GetValue(t) );
            if (t < cur_param->GetNumValues()-1)
               oss << " ";
            }
           ParamValue[count]->Text=oss.str().c_str();
           ParamValue[count]->Width=VALUE_WIDTHX;
           ParamValue[count]->ReadOnly=false;
           ParamValue[count]->Visible=true;
           ParamValue[count]->Parent=CfgTabControl;
           }
#ifdef TRY_PARAM_INTERPRETATION
        else
        {
          ParamInterpretation interpretation( *cur_param );
          if( interpretation.Kind() == ParamInterpretation::singleValuedEnum )
          {
            TComboBox* comboBox = new TComboBox( this );
            ParamComboBox[ count ] = comboBox;
            comboBox->Left = VALUE_OFFSETX;
            comboBox->Top= VALUE_OFFSETY + count * VALUE_SPACINGY;
            comboBox->Width = VALUE_WIDTHX;
            comboBox->Visible = true;
            comboBox->Sorted = false;
            comboBox->Style = csDropDownList;
            comboBox->Parent = CfgTabControl;
            for( size_t i = 0; i < interpretation.Values().size(); ++i )
              comboBox->Items->Add( interpretation.Values()[ i ].c_str() );
            comboBox->ItemIndex = ::atoi( cur_param->GetValue() ) - interpretation.IndexBase();
            comboBox->Hint = cur_param->GetComment();
            comboBox->ShowHint = true;
            ParamComment[ count ]->Caption = interpretation.Comment().c_str();
          }
          else if( interpretation.Kind() == ParamInterpretation::singleValuedBoolean )
          {
            TCheckBox* checkBox = new TCheckBox( this );
            ParamCheckBox[ count ] = checkBox;
            checkBox->Left = VALUE_OFFSETX;
            checkBox->Top= VALUE_OFFSETY + count * VALUE_SPACINGY;
            checkBox->Width = VALUE_WIDTHX;
            checkBox->Visible = true;
            checkBox->Parent = CfgTabControl;
            checkBox->Checked = ::atoi( cur_param->GetValue() );
            checkBox->Caption = interpretation.Comment().c_str();
            checkBox->Hint = cur_param->GetComment();
            checkBox->ShowHint = true;
            ParamComment[ count ]->Caption = "";
          }
#endif // TRY_PARAM_INTERPRETATION
        else
           {
           ParamValue[count]=new TEdit(this);
           ParamValue[count]->Left=VALUE_OFFSETX;
           ParamValue[count]->Top=VALUE_OFFSETY+count*VALUE_SPACINGY;
           ParamValue[count]->Text=cur_param->GetValue();
           ParamValue[count]->Width=VALUE_WIDTHX;
           ParamValue[count]->ReadOnly=false;
           ParamValue[count]->Visible=true;
           ParamValue[count]->Parent=CfgTabControl;
           }
#ifdef TRY_PARAM_INTERPRETATION
         }
#endif // TRY_PARAM_INTERPRETATION
        count++;
        }
     }
  }

#if 0 // jm 5/03
 if (count > 0)
    if (ParamLabel[count-1]->Top+ParamLabel[count-1]->Height > CfgTabControl->Height)
       {
       CfgTabControl->Height=ParamLabel[count-1]->Top+ParamLabel[count-1]->Height+15;
       // fConfig->Width=CfgTabControl->Width+40;
       }
#else
  if( count > 0 )
  {
    int bottomLine = 0;
    if( ParamUserLevel[ count - 1 ] != NULL )
      bottomLine = ParamUserLevel[ count - 1 ]->Top + ParamUserLevel[ count - 1 ]->Height;
    else if( ParamLabel[ count - 1 ] != NULL )
      bottomLine = ParamLabel[ count - 1 ]->Top + 3 * ParamLabel[ count - 1 ]->Height;
    if( bottomLine > 0 )
      CfgTabControl->Height = CfgTabControl->Height + bottomLine - CfgTabControl->DisplayRect.Bottom;
  }
#endif // jm 5/03

 cur_numparamsrendered=count;
 return(0);
}


// update one particular parameter on the screen
// useful, for example, if parameters change while stuff on screen
void TfConfig::RenderParameter(PARAM *cur_param)
{
AnsiString      valueline;
int     count, t;

 if (!Visible) return;

 // go through all the parameters
 // if one of the parameters one the screen has the same name as the parameter that we're supposed to update
 // then update the display
 for (count=0; count<cur_numparamsrendered; count++)
  {
  if (ParamLabel[count]->Caption == AnsiString(cur_param->GetName()))
     if (ParamValue[count])
        {
           ostringstream oss;
           for (size_t t=0; t<cur_param->GetNumValues(); t++)
            {
            oss << PARAM::encodedString( cur_param->GetValue(t) );
            if (t < cur_param->GetNumValues()-1)
               oss << " ";
            }
           ParamValue[count]->Text=oss.str().c_str();
        }
#ifdef TRY_PARAM_INTERPRETATION
     else if( ParamComboBox[ count ] )
       ParamComboBox[ count ]->ItemIndex = ::atoi( cur_param->GetValue() )
                                - ParamInterpretation( *cur_param ).IndexBase();
     else if( ParamCheckBox[ count ] )
       ParamCheckBox[ count ]->Checked = ::atoi( cur_param->GetValue() );
#endif // TRY_PARAM_INTERPRETATION
  }

 // in case we have a matrix on the screen, update the display, too
 // here, it can happen that we changed the matrix parameter, but it is being overwritten by the incoming matrix
 if ((fEditMatrix->Visible) && (fEditMatrix->GetDisplayedParamName() == cur_param->GetName()))
    fEditMatrix->SetDisplayedParam( cur_param );
}


// go through the parameters on the screen and update the parameters using the data on the screen
int TfConfig::UpdateParameters(AnsiString section)
{
int     count=0, i, t, u, num_param, num_values, ptr, endcount;
PARAM   *cur_param, *param;
AnsiString      paramname, paramvalue;
char            buf[2048];

 num_param=paramlist->GetNumParameters();

 // count the number of parameters on the sheet
 for (i=0; i<num_param; i++)
  {
  cur_param=paramlist->GetParamPtr(i);
  if (cur_param)
     if (section == cur_param->GetSection())
        count++;
  }

  // go through all parameters on the sheet
  // and update the parameters in the parameter list accordingly
  for (i=0; i<count; i++)
  {
    paramname=ParamLabel[i]->Caption;
    if (ParamValue[i])    // if we did not have an edit field, we had, e.g., a matrix and that is already updated
    {
       paramvalue=ParamValue[i]->Text.Trim();
       param=NULL;
       // search for the parameter with the same name in the parameterlist
       for (t=0; t<num_param; t++)
       {
        cur_param=paramlist->GetParamPtr(t);
        if (cur_param)
           if (paramname == cur_param->GetName())
              param=cur_param;
       }
       if (param)
       {
          if( string( param->GetType() ).find( "list" ) != string::npos )
          {
            istringstream is( paramvalue.c_str() );
            PARAM::encodedString value;
            int index = 0;
            while( is >> value )
              param->SetValue( value, index++ );
            param->SetNumValues( index );
          }
          else
            param->SetValue( paramvalue.c_str() );
       }
    }
#ifdef TRY_PARAM_INTERPRETATION
    else if( ParamComboBox[ i ] )
    {
      PARAM* param = paramlist->GetParamPtr( paramname.c_str() );
      if( param )
        param->SetValue( AnsiString( ParamComboBox[ i ]->ItemIndex
                         + ParamInterpretation( *param ).IndexBase() ).c_str() );
    }
    else if( ParamCheckBox[ i ] )
    {
      PARAM* param = paramlist->GetParamPtr( paramname.c_str() );
      if( param )
        param->SetValue( ParamCheckBox[ i ]->Checked ? "1" : "0" );
    }
#endif // TRY_PARAM_INTERPRETATION
  }

 return(0);
}


void __fastcall TfConfig::CfgTabControlChange(TObject *Sender)
{
 RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::FormClose(TObject *Sender, TCloseAction &Action)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
 DeleteAllParameters();
 DeleteAllTabs();
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::CfgTabControlChanging(TObject *Sender,
      bool &AllowChange)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
 AllowChange = true;
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bSaveParametersClick(TObject *Sender)
{
bool    ret;

 if (SaveDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
    fShowParameters->UpdateParameterTags(paramlist, 2);                         // update the tag (= filter) values in each parameter
    ret=paramlist->SaveParameterList(SaveDialog->FileName.c_str(), true);       // save parameters using the filter
    if (!ret)
       Application->MessageBox("Error writing parameter file", "Error", MB_OK);
    }
 else
    return;
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bLoadParametersClick(TObject *Sender)
{
bool    ret;

 LoadDialog->DefaultExt=".prm";
 LoadDialog->Filter="BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*";
 if (LoadDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
    fShowParameters->UpdateParameterTags(paramlist, 1);    // update the tag (= filter) values in each parameter
    ret=paramlist->LoadParameterList(LoadDialog->FileName.c_str(), true, false);        // do not import non-existing parameters; use the filter
    if (!ret)
       Application->MessageBox("Error reading parameter file", "Error", MB_OK);
    else
       RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
    }
 else
    return;
}


// **************************************************************************
// Function:   OnUserLevelChange
// Purpose:    Responds to a change to the user level
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void __fastcall TfConfig::OnUserLevelChange(TObject *Sender)
{
AnsiString      paramname;
PARAM   *param;
int     i;

 // find out which user level thingy the click came from
 // the track bar tells us about the parameter name
 paramname="";
 for (i=0; i<cur_numparamsrendered; i++)
  {
  if (ParamUserLevel[i] == Sender)
     paramname=ParamLabel[i]->Caption;
  }
 // now, given the parameter name, find it's pointer
 param=paramlist->GetParamPtr(paramname.c_str());
 if (!param)
    {
    Application->MessageBox("Could not find parameter", "Internal Error", MB_OK);
    return;
    }

 SetUserLevel(param, ((TTrackBar *)Sender)->Position);
}


// **************************************************************************
// Function:   bEditMatrixClick
// Purpose:    Responds to a click on the 'Load Matrix' button
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void __fastcall TfConfig::bEditMatrixClick(TObject *Sender)
{
AnsiString      paramname;
PARAM   *matrix_param;
int     i;

 // find out which button the click came from
 // the button tells us about the parameter name
 paramname="";
 for (i=0; i<cur_numparamsrendered; i++)
  {
  if (ParamButton[0][i] == Sender)
     paramname=ParamLabel[i]->Caption;
  }
 // now, given the parameter name, find it's pointer
 matrix_param=paramlist->GetParamPtr(paramname.c_str());
 if (!matrix_param)
    {
    Application->MessageBox("Could not find parameter", "Internal Error", MB_OK);
    return;
    }

 fEditMatrix->SetDisplayedParam( matrix_param );
 fEditMatrix->ShowModal();
}


// **************************************************************************
// Function:   bLoadMatrixClick
// Purpose:    Responds to a click on the 'Load Matrix' button
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void __fastcall TfConfig::bLoadMatrixClick(TObject *Sender)
{
AnsiString      paramname, new_matrix;
PARAM   *matrix_param;
int     ret;
int     i;

 // find out which button the click came from
 // the button tells us about the parameter name
 paramname="";
 for (i=0; i<cur_numparamsrendered; i++)
  {
  if (ParamButton[1][i] == Sender)
     paramname=ParamLabel[i]->Caption;
  }
 // now, given the parameter name, find it's pointer
 matrix_param=paramlist->GetParamPtr(paramname.c_str());
 if (!matrix_param)
    {
    Application->MessageBox("Could not find parameter", "Internal Error", MB_OK);
    return;
    }

 LoadDialog->DefaultExt=".mat";
 LoadDialog->Filter="Space delimited matrix file (*.mat)|*.mat|Any file (*.*)|*.*";
 if (LoadDialog->Execute())
    {
    ret=LoadMatrix(LoadDialog->FileName, matrix_param);
    if (ret != ERR_NOERR)
       {
       if (ret == ERR_MATLOADCOLSDIFF)
          Application->MessageBox("Number of columns in rows is different", "Error", MB_OK);
       if (ret == ERR_MATNOTFOUND)
          Application->MessageBox("Could not open matrix data file", "Error", MB_OK);
       if ((ret != ERR_MATNOTFOUND) && (ret != ERR_MATLOADCOLSDIFF))
          Application->MessageBox("Error loading the matrix file", "Error", MB_OK);
       return;
       }
    }
}


// **************************************************************************
// Function:   LoadMatrix
// Purpose:    Loads a matrix that is delimited by white spaces
// Parameters: matfilename   - filename of the matrix file, containing the full path
//             *matrixstring - pointer to the string that contains the matrix
// Returns:    ERR_NOERR - no error
//             ERR_MATLOADCOLSDIFF - number of columns in different rows is different
//             ERR_MATNOTFOUND - could not open input matrix file
// **************************************************************************
int TfConfig::LoadMatrix(AnsiString matfilename, PARAM *mat_param)
{
char    line[200000], buf[256];
FILE    *fp;
int     ptr, rows, cols, cur_cols, retcode;
int     cur_col, cur_row;

 fp=fopen(matfilename.c_str(), "rb");
 if (!fp) return(ERR_MATNOTFOUND);

 // first pass; parsing
 // find out rows and columns
 // check for consistency
 rows=cols=0;
 retcode=ERR_NOERR;
 while (!feof(fp))
  {
  fgets(line, 200000, fp);
  if (AnsiString(line).Trim().Length() > 0)
     {
     rows++;
     ptr=0;
     cur_cols=0;
     while (ptr < (int)strlen(line))
      {
      ptr=get_argument(ptr, buf, line, strlen(line));
      if (strlen(buf) != 0)
         cur_cols++;
      else
         break;
      }
     if ((cur_cols != cols) && (rows != 1))
        retcode=ERR_MATLOADCOLSDIFF;
     cols=cur_cols;
     }
  }

 // if there was an error, e.g., inconsistent number of columns, return w/o setting the new matrix
 if (retcode != ERR_NOERR)
    return(retcode);

 // set the dimensions of the new matrix size
 mat_param->SetDimensions(rows, cols);

 fseek(fp, 0, SEEK_SET);
 cur_row=cur_col=0;
 while (!feof(fp))
  {
  fgets(line, 200000, fp);
  if (AnsiString(line).Trim().Length() > 0)
     {
     ptr=0;
     cur_col=0;
     while (ptr < (int)strlen(line))
      {
      ptr=get_argument(ptr, buf, line, strlen(line));
      if (strlen(buf) != 0)
         {
         mat_param->SetValue(buf, cur_row, cur_col);
         cur_col++;
         }
      else
         break;
      }
     cur_row++;
     }
  }

 if (fp) fclose(fp);

return(ERR_NOERR);
}


// **************************************************************************
// Function:   get_argument
// Purpose:    parses a line of ASCII
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int TfConfig::get_argument(int ptr, char *buf, char *line, int maxlen)
{
 // skip trailing spaces, if any
 while ((line[ptr] == '=') || (line[ptr] == ' ') && (ptr < maxlen))
  ptr++;

 // go through the string, until we either hit a space, a '=', or are at the end
 while ((line[ptr] != '=') && (line[ptr] != ' ') && (line[ptr] != '\n') && (line[ptr] != '\r') && (ptr < maxlen))
  {
  *buf=line[ptr];
  ptr++;
  buf++;
  }

 *buf=0;
 return(ptr);
}




void __fastcall TfConfig::bConfigureSaveFilterClick(TObject *Sender)
{
 fShowParameters->parameterlist=paramlist;
 fShowParameters->filtertype=2;                 // filter for saving parameters
 fShowParameters->Caption="Save Filter";
 Application->MessageBox("The parameters that you select here will NOT be saved !", "Reminder", MB_OK);
 fShowParameters->ShowModal();
}
//---------------------------------------------------------------------------


void __fastcall TfConfig::bConfigureLoadFilterClick(TObject *Sender)
{
 fShowParameters->parameterlist=paramlist;
 fShowParameters->filtertype=1;                 // filter for loading parameters
 fShowParameters->Caption="Load Filter";
 Application->MessageBox("The parameters that you select here will NOT be loaded !", "Reminder", MB_OK);
 fShowParameters->ShowModal();
}
//---------------------------------------------------------------------------

#ifdef TRY_PARAM_INTERPRETATION
TfConfig::ParamInterpretation::ParamInterpretation( const PARAM& p )
: mIndexBase( 0 ),
  mKind( unknown ),
  mComment( p.GetComment() )
{
  string paramType = p.GetType();
  if( paramType == "matrix" )
    mKind = matrixGeneric;
  else if( paramType.find( "list" ) != string::npos )
    mKind = listGeneric;
  else
    mKind = singleValuedGeneric;

  switch( mKind )
  {
    case singleValuedGeneric:
      if( TryEnumInterpretation( p ) )
      {
        if( IsBooleanEnum() )
          mKind = singleValuedBoolean;
        else
          mKind = singleValuedEnum;
      }
      break;
    case listGeneric:
    case matrixGeneric:
      break;
    default:
      assert( false );
  }
}

bool
TfConfig::ParamInterpretation::TryEnumInterpretation( const PARAM& p )
{
  // Only int type parameters can be enumerations or booleans.
  const string enumParamType = "int";
  if( p.GetType() != enumParamType )
    return false;

  // Enumerations need a finite range.
  int lowRange = ::atoi( p.GetLowRange() ),
      highRange = ::atoi( p.GetHighRange() ),
      paramValue = ::atoi( p.GetValue() );
  if( lowRange != 0 && lowRange != 1
      || highRange <= lowRange
      || paramValue < lowRange
      || paramValue > highRange )
    return false;

  // Examine the comment: Does it contain an enumeration of all possible values?
  string comment = mComment;
  // Replace all punctuation marks with white space.
  const string cPunctuationChars = ",;:=()[]";
  int punctuationPos = comment.find_first_of( cPunctuationChars );
  while( punctuationPos != string::npos )
  {
    comment[ punctuationPos ] = ' ';
    punctuationPos = comment.find_first_of( cPunctuationChars );
  }

  map<int, int> histogram;
  istringstream is( comment );
  string        value,
                modifiedComment,
        *       currentLabel = &modifiedComment;
  while( is >> value )
  {
    // Using the >> operator for an int would accept "+" and similar strings as numbers.
    // We are only interested in groups of decimal digits.
    bool isNum = true;
    int numValue = 0;
    for( string::iterator i = value.begin(); isNum && i != value.end(); ++i )
    {
      if( *i < '0' || *i > '9' )
        isNum = false;
      else
      {
        numValue *= 10;
        numValue += *i - '0';
      }
    }
    if( isNum )
    {
      unsigned int index = numValue - lowRange;
      histogram[ index ]++;
      if( mValues.size() <= index )
        mValues.resize( index + 1 );
      currentLabel = &mValues[ index ];
    }
    else
    {
      if( !currentLabel->empty() )
        *currentLabel += " ";
      *currentLabel += value;
    }
  }

  bool isEnum = is.eof();

  // Each non-null value must be explained in the comment, thus appear exactly
  // once -- if in doubt, let's better return.
  for( size_t i = 1; isEnum && i < mValues.size(); ++i )
    if( histogram[ i ] != 1 )
      isEnum = false;

  // We consider this a boolean parameter.
  if( isEnum && lowRange == 0 && highRange == 1
      && histogram[ 0 ] == 0 && histogram[ 1 ] == 1 )
  {
    if( mValues.size() > 1 )
      modifiedComment = mValues[ 1 ];
    mValues.resize( 2 );
    mValues[ 0 ] = "no";
    mValues[ 1 ] = "yes";
  }

  if( mValues.size() != size_t( highRange - lowRange + 1 ) )
    isEnum = false;

  if( isEnum && mValues.size() > 0 && mValues[ 0 ] == "" )
    mValues[ 0 ] = "none";

  // Each of the other labels must now be non-empty.
  for( size_t i = 1; isEnum && i < mValues.size(); ++i )
    if( mValues[ i ].empty() )
      isEnum = false;

  if( isEnum )
  {
    mIndexBase = lowRange;
    mComment = modifiedComment;
    mKind = singleValuedEnum;
  }
  else
    mValues.clear();
  return isEnum;
}

bool
TfConfig::ParamInterpretation::IsBooleanEnum() const
{
  if( mIndexBase != 0 )
    return false;
  if( mValues.size() != 2 )
    return false;
  if( mValues[ 0 ] != "no" && mValues[ 0 ] != "No" )
    return false;
  if( mValues[ 1 ] != "yes" && mValues[ 1 ] != "Yes" )
    return false;
  return true;
}
#endif // TRY_PARAM_INTERPRETATION

#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <shlobj.h>
#include <Registry.hpp>
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <assert>

#include "..\shared\defines.h"
#include "UShowParameters.h"
#include "UEditMatrix.h"
#include "UOperatorCfg.h"
#include "UBCIError.h"
#include "UOperatorUtils.h"

#define ANYFILE_FILTER "All files (*.*)|*.*"
#define MATRIX_EXTENSION ".txt"
#define MATRIX_FILTER "Space delimited matrix file (*" MATRIX_EXTENSION ")"  \
                                       "|*" MATRIX_EXTENSION "|" ANYFILE_FILTER
using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfConfig *fConfig;
//---------------------------------------------------------------------------

__fastcall TfConfig::TfConfig(TComponent* Owner) : TForm(Owner)
{
  OperatorUtils::RestoreControl( this );
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
  for( size_t i = 0; i < mParamValues.size(); ++i )
    delete mParamValues[ i ];
  mParamValues.clear();
  for( size_t i = 0; i < mParamLabels.size(); ++i )
    delete mParamLabels[ i ];
  mParamLabels.clear();
  mParamInterpretations.clear();
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
}

// render all parameters in a particular section on the screen
int TfConfig::RenderParameters(AnsiString section)
{
  DeleteAllParameters();
  for( size_t i = 0; i < paramlist->GetNumParameters(); ++i )
  {
    PARAM* p = paramlist->GetParamPtr( i );
    if( section == p->GetSection() && GetUserLevel( p ) <= preferences->UserLevel )
    {
      int idx = mParamLabels.size();
      ParamInterpretation interpretation( *p );
      mParamInterpretations.push_back( interpretation );
      TControl* valueControl = NULL;
      bool commentLine = true;
      switch( interpretation.Kind() )
      {
        case ParamInterpretation::singleEntryEnum:
          {
            TComboBox* comboBox = new TComboBox( static_cast<TComponent*>( NULL ) );
            comboBox->Width = VALUE_WIDTHX;
            comboBox->Sorted = false;
            comboBox->Style = csDropDownList;
            comboBox->Parent = CfgTabControl;
            for( size_t i = 0; i < interpretation.Values().size(); ++i )
              comboBox->Items->Add( interpretation.Values()[ i ].c_str() );
            comboBox->Hint = interpretation.Comment().c_str();
            comboBox->ShowHint = true;
            valueControl = comboBox;
          }
          break;

        case ParamInterpretation::singleEntryBoolean:
          {
            TCheckBox* checkBox = new TCheckBox( static_cast<TComponent*>( NULL ) );
            checkBox->Width = VALUE_WIDTHX;
            checkBox->Caption = interpretation.Comment().c_str();
            checkBox->Hint = interpretation.Comment().c_str();
            checkBox->ShowHint = true;
            commentLine = false;
            valueControl = checkBox;
          }
          break;

        case ParamInterpretation::singleEntryInputFile:
        case ParamInterpretation::singleEntryOutputFile:
        case ParamInterpretation::singleEntryDirectory:
          {
            TEdit* edit = new TEdit( static_cast<TComponent*>( NULL ) );
            edit->Left = VALUE_OFFSETX;
            edit->Width = VALUE_WIDTHX;
            edit->ReadOnly = false;
            edit->OnChange = SyncHint;
            valueControl = edit;
            TButton* button = new TButton( edit );
            button->Caption = "...";
            button->Left = edit->Left + edit->Width + BUTTON_HEIGHT / 2;
            button->Width = BUTTON_HEIGHT;
            button->Height = BUTTON_HEIGHT;
            button->Top = VALUE_OFFSETY + idx * VALUE_SPACINGY;
            button->OnClick = OnChooseFileClick;
            button->Parent = CfgTabControl;
            button->Tag = idx;
          }
          break;
          
        case ParamInterpretation::singleEntryGeneric:
        case ParamInterpretation::listGeneric:
          {
            TEdit* edit = new TEdit( static_cast<TComponent*>( NULL ) );
            edit->Width = VALUE_WIDTHX;
            edit->ReadOnly = false;
            edit->OnChange = SyncHint;
            valueControl = edit;
          }
          break;

        case ParamInterpretation::matrixGeneric:
          {
            TButton* editButton = new TButton( static_cast<TComponent*>( NULL ) );
            editButton->Caption = "Edit Matrix";
            editButton->Left = VALUE_OFFSETX;
            editButton->Width = BUTTON_WIDTH;
            editButton->Height = BUTTON_HEIGHT;
            editButton->OnClick = bEditMatrixClick;
            valueControl = editButton;

            TButton* loadButton = new TButton( editButton );
            loadButton->Caption = "Load Matrix";
            loadButton->Left = editButton->Left + editButton->Width + BUTTON_SPACING;
            loadButton->Top = VALUE_OFFSETY + idx * VALUE_SPACINGY;
            loadButton->Width = BUTTON_WIDTH;
            loadButton->Height = BUTTON_HEIGHT;
            loadButton->OnClick = bLoadMatrixClick;
            loadButton->Visible = true;
            loadButton->Parent = CfgTabControl;
            loadButton->Tag = idx;

            TButton* saveButton = new TButton( editButton );
            saveButton->Caption = "Save Matrix";
            saveButton->Left = loadButton->Left + loadButton->Width + BUTTON_SPACING;
            saveButton->Top = VALUE_OFFSETY + idx * VALUE_SPACINGY;
            saveButton->Width = BUTTON_WIDTH;
            saveButton->Height = BUTTON_HEIGHT;
            saveButton->OnClick = bSaveMatrixClick;
            saveButton->Visible = true;
            saveButton->Parent = CfgTabControl;
            saveButton->Tag = idx;
          }
          break;

        default:
          assert( false );
      }
      valueControl->Left = VALUE_OFFSETX;
      valueControl->Top = VALUE_OFFSETY + idx * VALUE_SPACINGY;
      valueControl->ShowHint = true;
      valueControl->Visible = true;
      valueControl->Parent = CfgTabControl;
      valueControl->Tag = idx;
      mParamValues.push_back( valueControl );

      // render the parameter's name
      TLabel* label = new TLabel( static_cast<TComponent*>( NULL ) );
      label->Left = LABELS_OFFSETX;
      label->Top = LABELS_OFFSETY + idx * LABELS_SPACINGY;
      label->Caption = p->GetName();
      label->Font->Style = TFontStyles() << fsBold;
      label->Visible = true;
      label->Hint = interpretation.Comment().c_str();
      label->ShowHint = true;
      label->Parent = CfgTabControl;
      mParamLabels.push_back( label );

      // render the parameter's comment
      if( commentLine )
      {
        TLabel* comment = new TLabel( label );
        comment->Left = COMMENT_OFFSETX;
        comment->Top = COMMENT_OFFSETY + idx * COMMENT_SPACINGY;
        comment->Caption = interpretation.Comment().c_str();
        comment->Hint = interpretation.Comment().c_str();
        comment->ShowHint = true;
        comment->Font->Style = TFontStyles() << fsItalic;
        comment->Visible = true;
        comment->Parent = CfgTabControl;
      }

      // render the parameter's User Level track bar
      // ONLY, if the current user level is "advanced"
      if( preferences->UserLevel == USERLEVEL_ADVANCED )
      {
        TTrackBar* userLevel = new TTrackBar( valueControl );
        userLevel->Left = USERLEVEL_OFFSETX;
        userLevel->Top = USERLEVEL_OFFSETY + idx * USERLEVEL_SPACINGY;
        userLevel->Width = USERLEVEL_WIDTHX;
        userLevel->Min = 1;
        userLevel->Max = 3;
        userLevel->Position = GetUserLevel( p );
        userLevel->PageSize = 1;
        userLevel->OnChange = OnUserLevelChange;
        userLevel->Visible = true;
        userLevel->Parent = CfgTabControl;
        userLevel->Tag = idx;
      }
      RenderParameter( idx );
    }
  }
  if( !mParamLabels.empty() )
  {
    TTrackBar* trackBar = new TTrackBar( static_cast<TComponent*>( NULL ) );
    TLabel* bottomLabel = *mParamLabels.rbegin();
    int bottomLine = bottomLabel->Top + bottomLabel->Height + trackBar->Height;
    if( bottomLine > CfgTabControl->Height )
      CfgTabControl->Height = bottomLine;
    delete trackBar;
  }
  return 0;
}

// update one particular parameter on the screen
// useful, for example, if parameters change while stuff on screen
void TfConfig::RenderParameter( PARAM *inParam )
{
  if( !Visible )
    return;

  int idx = -1;
  for( size_t i = 0; idx == -1 && i < mParamLabels.size(); ++i )
    if( mParamLabels[ i ]->Caption == inParam->GetName() )
      idx = i;
  if( idx > 0 )
    RenderParameter( idx );
  // in case we have a matrix on the screen, update the display, too
  // here, it can happen that we changed the matrix parameter, but it is being overwritten by the incoming matrix
  if( fEditMatrix->Visible && ( fEditMatrix->GetDisplayedParamName() == inParam->GetName() ) )
    fEditMatrix->SetDisplayedParam( inParam );
}

void TfConfig::RenderParameter( int inIndex )
{
  PARAM* param = paramlist->GetParamPtr( mParamLabels[ inIndex ]->Caption.c_str() );
  TControl* value = mParamValues[ inIndex ];
  switch( mParamInterpretations[ inIndex ].Kind() )
  {
    case ParamInterpretation::singleEntryEnum:
      {
        TComboBox* comboBox = dynamic_cast<TComboBox*>( value );
        comboBox->ItemIndex =
          ::atoi( param->GetValue() ) - mParamInterpretations[ inIndex ].IndexBase();
      }
      break;

    case ParamInterpretation::singleEntryBoolean:
      {
        TCheckBox* checkBox = dynamic_cast<TCheckBox*>( value );
        checkBox->Checked = ::atoi( param->GetValue() );
      }
      break;

    case ParamInterpretation::singleEntryInputFile:
    case ParamInterpretation::singleEntryOutputFile:
    case ParamInterpretation::singleEntryDirectory:
    case ParamInterpretation::singleEntryGeneric:
      {
        TEdit* edit = dynamic_cast<TEdit*>( value );
        edit->Text = param->GetValue();
        edit->Hint = edit->Text;
      }
      break;

    case ParamInterpretation::listGeneric:
      {
        TEdit* edit = dynamic_cast<TEdit*>( value );
        ostringstream oss;
        oss << PARAM::encodedString( param->GetValue( 0 ) );
        for( size_t t = 1; t < param->GetNumValues(); ++t )
          oss << ' ' << PARAM::encodedString( param->GetValue( t ) );
        edit->Text = oss.str().c_str();
        edit->Hint = edit->Text;
      }
      break;

    case ParamInterpretation::matrixGeneric:
      break;

    default:
      assert( false );
  }
}

// go through the parameters on the screen and update the parameters using the data on the screen
int TfConfig::UpdateParameters()
{
  for( size_t i = 0; i < mParamLabels.size(); ++i )
  {
    PARAM* param = paramlist->GetParamPtr( mParamLabels[ i ]->Caption.c_str() );
    TControl* valueControl = mParamValues[ i ];
    switch( mParamInterpretations[ i ].Kind() )
    {
      case ParamInterpretation::singleEntryEnum:
        {
          TComboBox* comboBox = dynamic_cast<TComboBox*>( valueControl );
          ostringstream oss;
          oss << comboBox->ItemIndex + mParamInterpretations[ i ].IndexBase();
          param->SetValue( oss.str().c_str() );
        }
        break;

      case ParamInterpretation::singleEntryBoolean:
        {
          TCheckBox* checkBox = dynamic_cast<TCheckBox*>( valueControl );
          param->SetValue( checkBox->Checked ? "1" : "0" );
        }
        break;

      case ParamInterpretation::singleEntryInputFile:
      case ParamInterpretation::singleEntryOutputFile:
      case ParamInterpretation::singleEntryDirectory:
      case ParamInterpretation::singleEntryGeneric:
        {
          TEdit* edit = dynamic_cast<TEdit*>( valueControl );
          param->SetValue( edit->Text.Trim().c_str() );
        }
        break;

      case ParamInterpretation::listGeneric:
        {
          TEdit* edit = dynamic_cast<TEdit*>( valueControl );
          istringstream is( edit->Text.c_str() );
          PARAM::encodedString value;
          int index = 0;
          while( is >> value )
            param->SetValue( value, index++ );
          param->SetNumValues( index );
        }
        break;

      case ParamInterpretation::matrixGeneric:
        break;

      default:
        assert( false );
    }
  }
  return 0;
}


void __fastcall TfConfig::CfgTabControlChange(TObject *Sender)
{
 RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::FormClose(TObject *Sender, TCloseAction &Action)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters();
 DeleteAllParameters();
 DeleteAllTabs();
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::CfgTabControlChanging(TObject *Sender,
      bool &AllowChange)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters();
 AllowChange = true;
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bSaveParametersClick(TObject *Sender)
{
bool    ret;

 if (SaveDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters();
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
       UpdateParameters();
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
  TTrackBar* trackBar = dynamic_cast<TTrackBar*>( Sender );
  AnsiString paramname = mParamLabels[ trackBar->Tag ]->Caption;
  PARAM* param = paramlist->GetParamPtr( paramname.c_str() );
  if( param )
    SetUserLevel( param, trackBar->Position );
  else
    Application->MessageBox( "Could not find parameter", "Internal Error", MB_OK );
}


// **************************************************************************
// Function:   bEditMatrixClick
// Purpose:    Responds to a click on the 'Load Matrix' button
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void __fastcall TfConfig::bEditMatrixClick(TObject *Sender)
{
  TControl* control = dynamic_cast<TControl*>( Sender );
  AnsiString paramname = mParamLabels[ control->Tag ]->Caption;
  PARAM* matrix_param = paramlist->GetParamPtr( paramname.c_str() );
  if( matrix_param )
  {
    fEditMatrix->SetDisplayedParam( matrix_param );
    fEditMatrix->ShowModal();
  }
  else
    Application->MessageBox( "Could not find parameter", "Internal Error", MB_OK );
}


// **************************************************************************
// Function:   bLoadMatrixClick
// Purpose:    Responds to a click on the 'Load Matrix' button
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void
__fastcall
TfConfig::bLoadMatrixClick( TObject* inSender )
{
  TButton* button = dynamic_cast<TButton*>( inSender );
  PARAM* param = paramlist->GetParamPtr( mParamLabels[ button->Tag ]->Caption.c_str() );
  if( param == NULL )
  {
    Application->MessageBox( "Could not find parameter", "Internal Error", MB_OK );
    return;
  }

  LoadDialog->DefaultExt = MATRIX_EXTENSION;
  LoadDialog->Filter = MATRIX_FILTER;
  LoadDialog->Options << ofFileMustExist;
  if( LoadDialog->Execute() )
  {
    int result = LoadMatrix( LoadDialog->FileName, param );
    switch( result )
    {
      case ERR_NOERR:
        break;
      case ERR_MATLOADCOLSDIFF:
        Application->MessageBox( "Number of columns in rows is different", "Error", MB_OK );
        break;
      case ERR_MATNOTFOUND:
        Application->MessageBox( "Could not open matrix data file", "Error", MB_OK );
        break;
      default:
        Application->MessageBox("Error loading the matrix file", "Error", MB_OK);
    }
  }
}

// **************************************************************************
// Function:   bSaveMatrixClick
// Purpose:    Responds to a click on the 'Save Matrix' button
// Parameters: Sender - pointer to the sending object
// Returns:    N/A
// **************************************************************************
void
__fastcall
TfConfig::bSaveMatrixClick( TObject* inSender )
{
  TButton* button = dynamic_cast<TButton*>( inSender );
  PARAM* param = paramlist->GetParamPtr( mParamLabels[ button->Tag ]->Caption.c_str() );
  if( param == NULL )
  {
    Application->MessageBox( "Could not find parameter", "Internal Error", MB_OK );
    return;
  }

  SaveDialog->DefaultExt = MATRIX_EXTENSION;
  SaveDialog->Filter = MATRIX_FILTER;
  if( SaveDialog->Execute() )
  {
    int result = SaveMatrix( SaveDialog->FileName, param );
    switch( result )
    {
      case ERR_NOERR:
        break;
      case ERR_COULDNOTWRITE:
      default:
        {
          AnsiString message;
          message = "Could not write to file ";
          message += SaveDialog->FileName;
          Application->MessageBox( message.c_str(), "Error", MB_OK );
        }
        break;
    }
  }
}

// **************************************************************************
// Function:   LoadMatrix
// Purpose:    Loads a matrix that is delimited by white spaces
// Parameters: - filename of the matrix file, containing the full path
//             - pointer to the parameter that contains the matrix
// Returns:    ERR_NOERR - no error
//             ERR_MATLOADCOLSDIFF - number of columns in different rows is different
//             ERR_MATNOTFOUND - could not open input matrix file or file contains no data
// **************************************************************************
int
TfConfig::LoadMatrix( const AnsiString& inFileName, PARAM* inParam ) const
{
  vector<vector<string> > matrix;
  
  ifstream input( inFileName.c_str() );
  string line;
  while( getline( input, line ) )
  {
    istringstream is( line );
    vector<string> row;
    string value;
    while( is >> value )
      row.push_back( value );
    if( !row.empty() )
      matrix.push_back( row );
  }
  if( matrix.empty() )
    return ERR_MATNOTFOUND;

  size_t numRows = matrix.size(),
         numCols = matrix[ 0 ].size();
  for( size_t row = 1; row < numRows; ++row )
    if( matrix[ row ].size() != numCols )
      return ERR_MATLOADCOLSDIFF;

  inParam->SetDimensions( numRows, numCols );
  for( size_t row = 0; row < numRows; ++row )
    for( size_t col = 0; col < numCols; ++col )
      inParam->SetValue( matrix[ row ][ col ], row, col );

  return ERR_NOERR;
}

// **************************************************************************
// Function:   SaveMatrix
// Purpose:    Saves a matrix to a file, delimited by white spaces
// Parameters: - filename of the matrix file, containing the full path
//             - pointer to the parameter that contains the matrix
// Returns:    ERR_NOERR - no error
//             ERR_COULDNOTWRITE - could not write matrix to output file
// **************************************************************************
int
TfConfig::SaveMatrix( const AnsiString& inFileName, PARAM* inParam ) const
{
  ofstream output( inFileName.c_str() );
  for( size_t row = 0; row < inParam->GetNumValuesDimension1(); ++row )
  {
    for( size_t col = 0; col < inParam->GetNumValuesDimension2(); ++col )
      output << ' ' << setw( 8 ) << inParam->GetValue( row, col );
    output << endl;
  }
  return output ? ERR_NOERR : ERR_COULDNOTWRITE;
}
//---------------------------------------------------------------------------

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

void __fastcall TfConfig::OnChooseFileClick( TObject* inSender )
{
  TButton*   button = dynamic_cast<TButton*>( inSender );
  TEdit*     edit = dynamic_cast<TEdit*>( mParamValues[ button->Tag ] );
  AnsiString comment = mParamInterpretations[ button->Tag ].Comment().c_str();
  switch( mParamInterpretations[ button->Tag ].Kind() )
  {
    case ParamInterpretation::singleEntryInputFile:
      {
        TOpenDialog* dialog = new TOpenDialog( static_cast<TComponent*>( NULL ) );
        dialog->Title = AnsiString( "Choosing " ) + comment;
        dialog->InitialDir = ::ExtractFilePath( edit->Text );
        dialog->DefaultExt = "";
        dialog->Filter = ANYFILE_FILTER;
        dialog->Options.Clear();
        dialog->Options << ofFileMustExist << ofHideReadOnly
                        << ofNoChangeDir << ofDontAddToRecent;
        if( dialog->Execute() )
          edit->Text = dialog->FileName;
        delete dialog;
      }
      break;
    case ParamInterpretation::singleEntryOutputFile:
      {
        TSaveDialog* dialog = new TSaveDialog( static_cast<TComponent*>( NULL ) );
        dialog->Title = AnsiString( "Choosing " ) + comment;
        dialog->InitialDir = ::ExtractFilePath( edit->Text );
        dialog->DefaultExt = "";
        dialog->Filter = ANYFILE_FILTER;
        dialog->Options.Clear();
        dialog->Options << ofPathMustExist << ofHideReadOnly
                        << ofNoChangeDir << ofDontAddToRecent
                        << ofNoReadOnlyReturn << ofOverwritePrompt;
        if( dialog->Execute() )
          edit->Text = dialog->FileName;
        delete dialog;
      }
      break;
    case ParamInterpretation::singleEntryDirectory:
      {
        LPMALLOC shellMalloc;
        if( ::SHGetMalloc( &shellMalloc ) == NO_ERROR )
        {
          char buffer[ MAX_PATH ];
          BROWSEINFO browseinfo =
          {
            Handle,
            NULL,
            buffer,
            comment.c_str(),
            BIF_RETURNONLYFSDIRS,
            NULL, 0, NULL
          };
          ITEMIDLIST* result = ::SHBrowseForFolder( &browseinfo );
          if( result )
          {
            if( ::SHGetPathFromIDList( result, buffer ) )
              edit->Text = buffer;
            shellMalloc->Free( result );
          }
          shellMalloc->Release();
        }
      }
      break;
    default:
      assert( false );
  }
}

void __fastcall TfConfig::SyncHint( TObject* inSender )
{
  TEdit* edit = dynamic_cast<TEdit*>( inSender );
  edit->Hint = edit->Text;
}

TfConfig::ParamInterpretation::ParamInterpretation( const PARAM& p )
: mIndexBase( 0 ),
  mKind( unknown ),
  mComment( p.GetComment() )
{
  // Look for "interpretation hints" in the comment.
  // Unknown hints (syntax errors) will show up in the operator's comment display.
  const struct
  {
    const char* keyword;
    Kind_type   kind;
  } hints[] =
  {
    { "@enumeration", singleEntryEnum },
    { "@boolean",     singleEntryBoolean },
    { "@inputfile",   singleEntryInputFile },
    { "@outputfile",  singleEntryOutputFile },
    { "@directory",   singleEntryDirectory },
  };
  for( int i = 0; ( mKind == unknown ) && ( i < sizeof( hints ) / sizeof( *hints ) ); ++i )
  {
    size_t hintPos = mComment.find( hints[ i ].keyword );
    if( hintPos != string::npos )
    {
      mComment = mComment.erase( hintPos );
      mKind = hints[ i ].kind;
    }
  }
  // For matrix and list type parameters, the hint is ignored.
  string paramType = p.GetType();
  if( paramType.find( "matrix" ) != string::npos )
    mKind = matrixGeneric;
  else if( paramType.find( "list" ) != string::npos )
    mKind = listGeneric;
  else if( mKind == unknown )
    mKind = singleEntryGeneric;

  switch( mKind )
  {
    case singleEntryEnum:
      if( !ExtractEnumValues( p ) )
        mKind = singleEntryGeneric;
      break;

    case singleEntryBoolean:
      if( !( ExtractEnumValues( p ) && IsBooleanEnum() ) )
        mKind = singleEntryGeneric;
      break;

    case singleEntryInputFile:
    case singleEntryOutputFile:
    case singleEntryDirectory:
    case singleEntryGeneric:
    case listGeneric:
    case matrixGeneric:
      break;

    default:
      assert( false );
  }
}

bool
TfConfig::ParamInterpretation::ExtractEnumValues( const PARAM& p )
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




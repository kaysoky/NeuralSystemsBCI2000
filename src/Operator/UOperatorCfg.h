//---------------------------------------------------------------------------

#ifndef UOperatorCfgH
#define UOperatorCfgH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "USysStatus.h"
#include "UPreferences.h"
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <StdCtrls.hpp>

#include <vector>
#include <string>

#define MAX_PARAMPERSECTION     300
#define LABELS_OFFSETX          30
#define LABELS_OFFSETY          50
#define LABELS_SPACINGY         40
#define COMMENT_OFFSETX         170
#define COMMENT_OFFSETY         35
#define COMMENT_SPACINGY        40
#define VALUE_OFFSETX           COMMENT_OFFSETX
#define VALUE_OFFSETY           50
#define VALUE_SPACINGY          40
#define VALUE_WIDTHX            220
#define BUTTON_WIDTH            ( ( VALUE_WIDTHX - 20 ) / 3 )
#define BUTTON_HEIGHT           21
#define BUTTON_SPACING          ( ( VALUE_WIDTHX - 3 * BUTTON_WIDTH ) / 2 )
#define USERLEVEL_OFFSETX       VALUE_OFFSETX+260
#define USERLEVEL_WIDTHX        70
#define USERLEVEL_OFFSETY       50
#define USERLEVEL_SPACINGY      40

//---------------------------------------------------------------------------
class TfConfig : public TForm
{
__published:	// IDE-managed Components
        TTabControl *CfgTabControl;
        TButton *bLoadParameters;
        TButton *bSaveParameters;
        TOpenDialog *LoadDialog;
        TSaveDialog *SaveDialog;
        TButton *bConfigureSaveFilter;
        TButton *bConfigureLoadFilter;
        void __fastcall CfgTabControlChange(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall CfgTabControlChanging(TObject *Sender,
          bool &AllowChange);
        void __fastcall bSaveParametersClick(TObject *Sender);
        void __fastcall bLoadParametersClick(TObject *Sender);
        void __fastcall bEditMatrixClick(TObject *Sender);
        void __fastcall bLoadMatrixClick(TObject *Sender);
        void __fastcall bSaveMatrixClick(TObject *Sender);
        void __fastcall OnUserLevelChange(TObject *Sender);
        void __fastcall bConfigureSaveFilterClick(TObject *Sender);
        void __fastcall bConfigureLoadFilterClick(TObject *Sender);
private:
        void __fastcall OnChooseFileClick( TObject* );
        void __fastcall SyncHint( TObject* );
public:		// User declarations
        __fastcall TfConfig(TComponent* Owner);
        __fastcall ~TfConfig();
        int     Initialize(PARAMLIST *paramlist, PREFERENCES *);
        void    DeleteAllTabs();
        void    DeleteAllParameters();
        int     RenderParameters(AnsiString section);
        void    RenderParameter(PARAM *cur_param);
        void    RenderParameter(int idx);
        int     UpdateParameters();
        int     LoadMatrix( const AnsiString& matfilename, PARAM* ) const;
        int     SaveMatrix( const AnsiString& matfilename, PARAM* ) const;
        int     GetUserLevel(PARAM *param);
        void    SetUserLevel(PARAM *param, int cur_userlevel);
private:
        class ParamInterpretation
        {
          // Type declarations.
          public:
            // Possible interpretation results.
            typedef enum
            {
              unknown = 0,
              singleEntryGeneric,
                singleEntryEnum,      // Possible parameter values are from a pre-defined set.
                singleEntryBoolean,   // The parameter represents an on/off switch.
                                      // Logically, this is a specialization of singleValuedEnum.
                singleEntryInputFile, // Parameter values are full paths to files.
                singleEntryOutputFile,
                singleEntryDirectory, // Parameter values are directory paths.

              listGeneric,
              matrixGeneric,
            } Kind_type;
          private:
            typedef std::vector<std::string> Values_type;

          // The public interface.
          public:
            ParamInterpretation( const PARAM& );

            // The parameter comment as modified by the interpretation.
            const std::string& Comment() const   { return mComment; }

            Kind_type          Kind() const      { return mKind; }
            const Values_type& Values() const    { return mValues; }

            // This is only relevant for the singleValuedEnum type and represents
            // the numerical parameter value of the first enumeration entry.
            int                IndexBase() const { return mIndexBase; }

          // Private members.
          private:
            bool ExtractEnumValues( const PARAM& p );
            bool IsBooleanEnum() const;

            std::string mComment;
            Kind_type   mKind;
            Values_type mValues;
            int         mIndexBase;
        };

        enum error
        {
          ERR_MATLOADCOLSDIFF = 1,
          ERR_MATNOTFOUND,
          ERR_COULDNOTWRITE,
        };
        PREFERENCES* preferences;
        PARAMLIST  * paramlist;

        std::vector<TControl*>           mParamValues;
        std::vector<TLabel*>             mParamLabels;
        std::vector<ParamInterpretation> mParamInterpretations;
};
//---------------------------------------------------------------------------
extern PACKAGE TfConfig *fConfig;
//---------------------------------------------------------------------------
#endif

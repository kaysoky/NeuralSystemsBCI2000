//---------------------------------------------------------------------------

#ifndef UOperatorCfgH
#define UOperatorCfgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <CheckLst.hpp>

#include "UParameter.h"
#include "USysStatus.h"
#include "UPreferences.h"

#ifdef TRY_PARAM_INTERPRETATION
# include <vector>
# include <string> 
#endif // TRY_PARAM_INTERPRETATION

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
#define USERLEVEL_OFFSETX       VALUE_OFFSETX+260
#define USERLEVEL_WIDTHX        70
#define USERLEVEL_OFFSETY       50
#define USERLEVEL_SPACINGY      40

#define ERR_NOERR               0
#define ERR_MATLOADCOLSDIFF     1
#define ERR_MATNOTFOUND         2


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
        void __fastcall OnUserLevelChange(TObject *Sender);
        void __fastcall bConfigureSaveFilterClick(TObject *Sender);
        void __fastcall bConfigureLoadFilterClick(TObject *Sender);
private:	// User declarations
        PREFERENCES     *preferences;
        PARAMLIST       *paramlist;
      	TLabel	*ParamLabel[MAX_PARAMPERSECTION];
      	TLabel	*ParamComment[MAX_PARAMPERSECTION];
      	TEdit	*ParamValue[MAX_PARAMPERSECTION];
#ifdef TRY_PARAM_INTERPRETATION
        TComboBox* ParamComboBox[MAX_PARAMPERSECTION];
        TCheckBox* ParamCheckBox[MAX_PARAMPERSECTION];
#endif
      	TButton	*ParamButton[3][MAX_PARAMPERSECTION];
      	TTrackBar *ParamUserLevel[MAX_PARAMPERSECTION];
        int     cur_numparamsrendered;
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
public:		// User declarations
        __fastcall TfConfig(TComponent* Owner);
        __fastcall ~TfConfig();
        int     Initialize(PARAMLIST *paramlist, PREFERENCES *);
        void    DeleteAllTabs();
        void    DeleteAllParameters();
        int     RenderParameters(AnsiString section);
        void    RenderParameter(PARAM *cur_param);
        int     UpdateParameters(AnsiString section);
        int     LoadMatrix(AnsiString matfilename, PARAM *mat_param);
        int     GetUserLevel(PARAM *param);
        void    SetUserLevel(PARAM *param, int cur_userlevel);
#ifdef TRY_PARAM_INTERPRETATION
private:
        class ParamInterpretation
        {
          // Type declarations.
          public:
            // Possible interpretation results.
            typedef enum
            {
              unknown = 0,
              singleValuedGeneric,
                singleValuedEnum,         // Possible parameter values are from a pre-defined set.
                singleValuedBoolean,      // The parameter represents an on/off switch.
                                          // Logically, this is a specialization of singleValuedEnum.
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
            bool TryEnumInterpretation( const PARAM& p );
            bool IsBooleanEnum() const;

            std::string mComment;
            Kind_type   mKind;
            Values_type mValues;
            int         mIndexBase;
        };
#endif // TRY_PARAM_INTERPRETATION
};
//---------------------------------------------------------------------------
extern PACKAGE TfConfig *fConfig;
//---------------------------------------------------------------------------
#endif

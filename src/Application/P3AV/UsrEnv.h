/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef UserEnvH
#define UserEnvH

#include "UsrElementCollection.h"

/// forward declarations
class UsrEnvAlgorithm;
class PARAMLIST;
class TTask;

class UsrEnv
{
public:

  enum ElementCollEnum
  {
    BIT_GENERAL = 0,
    BIT_ACTIVE,
    // create flag values from bit numbers
    COLL_GENERAL = ( 1 << BIT_GENERAL ),
    COLL_ACTIVE  = ( 1 << BIT_ACTIVE ),
    COLL_ALL     = COLL_GENERAL | COLL_ACTIVE,
  };
  /// Constructors and Destructors
  UsrEnv::UsrEnv(const AnsiString & asTaskName, UsrEnvAlgorithm * pAlgorithm);
  UsrEnv::~UsrEnv();

  /// Sets and Gets
  // Set and get for active elements collection
  inline UsrElementCollection * GetActiveElements(void) { return m_pActiveUsrElementColl; }
  inline UsrElementCollection * GetElements(void) { return m_pUsrElementColl; }

  /// Member functions
  void Initialize(TTask * pTask, TApplication * pApplication, const int & iTop, const int & iLeft,
                  const int & iWidth,const int & iHeight, TColor color);
  void HideElements(const ElementCollEnum eElementColl);
  void ShowElements( ElementCollEnum inElementColl,
                     UsrElementCollection::SelectionModeEnum inSelectionMode,
                     unsigned int inElementID = 0 );
  void __fastcall RenderElements( TObject* );

  void DisplayMessage(const char * message);
  void HideMessage(void);
  void InitializeAlgorithm(TTask * pTask, TApplication * pApplication);
  const unsigned int GenerateActiveElements(const unsigned int & uPhaseInSequence);
  void SetWindowSize(const int & iTop, const int & iLeft,
                     const int & iWidth,const int & iHeight, TColor color);
  const int GetStateValue(const unsigned int & uState);

private:
  /// member variables
  UsrElementCollection * m_pUsrElementColl;
  UsrElementCollection * m_pActiveUsrElementColl;
  UsrEnvAlgorithm * m_pAlgorithm;
  TLabel * m_pMessage;
  class UsrEnvForm : public TForm
  {
    public:
      UsrEnvForm() : TForm( static_cast<TComponent*>(NULL), 1 ) {}
      __fastcall ~UsrEnvForm() {}

    private:
      // Avoid flicker by specifying an empty WMEraseBkgnd handler.
      void __fastcall WMEraseBkgnd( TWMEraseBkgnd& Message ) {}

    BEGIN_MESSAGE_MAP
      MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
    END_MESSAGE_MAP( TForm )

  }* m_pForm;
  
  Graphics::TBitmap* m_pOffscreenBuffer;

};
#endif



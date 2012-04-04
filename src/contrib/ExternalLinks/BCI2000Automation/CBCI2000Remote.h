#ifndef C_BCI2000_REMOTE_H
#define C_BCI2000_REMOTE_H

#include "BCI2000Automation.h"
#include "BCI2000Remote.h"
#include "ComCoClass.h"
#include "ComStrings.h"
#include "ComModule.h"
#include "Resource.h"

#define BOOLRESULT(x) *success = BCI2000Remote::x ? VARIANT_TRUE : VARIANT_FALSE;

class CBCI2000Remote : private BCI2000Remote,
  public com::CoClass<IBCI2000Remote, &IID_IBCI2000Remote, CBCI2000Remote, &CLSID_BCI2000Remote>
{
public:
    CBCI2000Remote()
    {
      wchar_t* pOperator = NULL;
      if( ::LoadStringW( com::Module::GetHInstance(), IDS_OperatorName, reinterpret_cast<LPWSTR>( &pOperator ), 0 ) )
      {
        com::DualString path = com::Module::GetLocation();
        path += pOperator;
        OperatorPath( std::string( path ) );
      }
    }

    virtual /* [helpstring][propget] */ HRESULT __stdcall get_Timeout(
        /* [retval][out] */ double *seconds)
        {
          *seconds = Timeout();
          return S_OK;
        }

    virtual /* [helpstring][propput] */ HRESULT __stdcall put_Timeout(
        /* [in] */ double seconds)
        {
          Timeout( seconds );
          return S_OK;
        }

    virtual /* [helpstring][propget] */ HRESULT __stdcall get_OperatorPath(
        /* [retval][out] */ BSTR *path)
        {
          *path = ::SysAllocString( com::DualString( OperatorPath() ).c_str() );
          return S_OK;
        }

    virtual /* [helpstring][propput] */ HRESULT __stdcall put_OperatorPath(
        /* [in] */ BSTR path)
        {
          OperatorPath( com::DualString( path ) );
          return S_OK;
        }

    virtual /* [helpstring][propget] */ HRESULT __stdcall get_TelnetAddress(
        /* [retval][out] */ BSTR *address)
        {
          *address = ::SysAllocString( com::DualString( TelnetAddress() ).c_str() );
          return S_OK;
        }

    virtual /* [helpstring][propput] */ HRESULT __stdcall put_TelnetAddress(
        /* [in] */ BSTR address)
        {
          TelnetAddress( com::DualString( address ) );
          return S_OK;
        }

    virtual /* [helpstring][propget][id] */ HRESULT __stdcall get_WindowVisible( 
        /* [retval][out] */ VARIANT_BOOL *visible)
        {
          *visible = WindowVisible() ? VARIANT_TRUE : VARIANT_FALSE;
          return S_OK;
        }

    virtual /* [helpstring][propput] */ HRESULT __stdcall put_WindowVisible( 
        /* [in] */ VARIANT_BOOL visible)
        {
          WindowVisible( visible );
          return S_OK;
        }
    
    virtual /* [helpstring][propget][id] */ HRESULT __stdcall get_WindowTitle( 
        /* [retval][out] */ BSTR *title)
        {
          *title = ::SysAllocString( com::DualString( WindowTitle() ).c_str() );
          return S_OK;
        }
        
    virtual /* [helpstring][propput] */ HRESULT __stdcall put_WindowTitle( 
        /* [in] */ BSTR title)
        {
          WindowTitle( com::DualString( title ) );
          return S_OK;
        }

    virtual /* [helpstring][propget] */ HRESULT __stdcall get_Result(
        /* [retval][out] */ BSTR *result)
        {
          *result = ::SysAllocString( com::DualString( Result() ).c_str() );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall Connect( 
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( Connect() );
          return S_OK;
        }
    
    virtual /* [helpstring] */ HRESULT __stdcall Disconnect( 
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( Disconnect() );
          return S_OK;
        }
    
    virtual /* [helpstring] */ HRESULT __stdcall ExecuteScriptCommand(
        /* [in] */ BSTR command,
        /* [retval][out] */ int *exitCode)
        {
          *exitCode = Execute( com::DualString( command ) );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall StartupModules(
        /* [in] */ SAFEARRAY * modules,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          int nDim = ::SafeArrayGetDim( modules );
          if( nDim < 1 )
            return E_INVALIDARG;
          LONG lBound = 0, uBound = 0;
          HRESULT result = ::SafeArrayGetLBound( modules, nDim, &lBound );
          if( FAILED( result ) )
            return result;
          result = ::SafeArrayGetUBound( modules, nDim, &uBound );
          if( FAILED( result ) )
            return result;
          std::vector<std::string> modules2;
          BSTR* pStrings = NULL;
          result = ::SafeArrayAccessData( modules, reinterpret_cast<void**>( &pStrings ) );
          if( FAILED( result ) )
            return result;
          for( LONG i = 0; i < uBound - lBound + 1; ++i )
            modules2.push_back( com::DualString( pStrings[i] ) );
          ::SafeArrayUnaccessData( modules );
          *success = BCI2000Remote::StartupModules( modules2 ) ? VARIANT_TRUE : VARIANT_FALSE;
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall LoadParametersLocal(
        /* [in] */ BSTR file,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( LoadParametersLocal( com::DualString( file ) ) );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall LoadParametersRemote(
        /* [in] */ BSTR file,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( LoadParametersRemote( com::DualString( file ) ) );
          return S_OK;
        }

    virtual /* [helpstring][propget] */ HRESULT __stdcall get_SubjectID( 
        /* [retval][out] */ BSTR *subjectID)
        {
          *subjectID = ::SysAllocString( com::DualString( SubjectID() ).c_str() );
          return S_OK;
        }
    
    virtual /* [helpstring][propput] */ HRESULT __stdcall put_SubjectID( 
        /* [in] */ BSTR subjectID)
        {
          SubjectID( com::DualString( subjectID ) );
          return S_OK;
        }
    
    virtual /* [helpstring][propget] */ HRESULT __stdcall get_SessionID( 
        /* [retval][out] */ BSTR *sessionID)
        {
          *sessionID = ::SysAllocString( com::DualString( SessionID() ).c_str() );
          return S_OK;
        }
    
    virtual /* [helpstring][propput] */ HRESULT __stdcall put_SessionID( 
        /* [in] */ BSTR sessionID)
        {
          SessionID( com::DualString( sessionID ) );
          return S_OK;
        }
    
    virtual /* [helpstring][propget] */ HRESULT __stdcall get_DataDirectory( 
        /* [retval][out] */ BSTR *directory)
        {
          *directory = ::SysAllocString( com::DualString( DataDirectory() ).c_str() );
          return S_OK;
        }
    
    virtual /* [helpstring][propput] */ HRESULT __stdcall put_DataDirectory( 
        /* [in] */ BSTR directory)
        {
          DataDirectory( com::DualString( directory ) );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall SetConfig(
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( SetConfig() );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall Start(
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( Start() );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall Stop(
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( Stop() );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall GetSystemState( 
        /* [out][in] */ BSTR *state,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          std::string buf;
          BOOLRESULT( GetSystemState( buf ) );
          *state = ::SysAllocString( com::DualString( buf ).c_str() );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall SetStateVariable( 
        /* [in] */ BSTR stateName,
        /* [in] */ double value,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( SetStateVariable( com::DualString( stateName ), value ) );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall GetStateVariable(
        /* [in] */ BSTR stateName,
        /* [out][in] */ double *value,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( GetStateVariable( com::DualString( stateName ), *value ) );
          return S_OK;
        }

    virtual /* [helpstring] */ HRESULT __stdcall GetControlSignal(
        /* [in] */ int channel,
        /* [in] */ int element,
        /* [out][in] */ double *value,
        /* [retval][out] */ VARIANT_BOOL *success)
        {
          BOOLRESULT( GetControlSignal( channel, element, *value ) );
          return S_OK;
        }
};

#endif // C_BCI2000_REMOTE_H

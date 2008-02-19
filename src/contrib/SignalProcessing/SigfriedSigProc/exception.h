// CbException.h: interface for the CCbException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CbExcepTION_H__5EB101FC_86AD_45C1_880A_B9514ABF2804__INCLUDED_)
#define AFX_CbExcepTION_H__5EB101FC_86AD_45C1_880A_B9514ABF2804__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/* consider

  
    LC_AS_STRING(__LINE__) 
    __FILE__ 
    __FUNCDNAME__ 
    __PRETTY_FUNCTION__
     __FUNCTION__ 

  */


typedef struct SIGFRIED_ERROR_STRUCT{
  int     errorcode;
  string  szerrorcode;
} SIGFRIED_ERROR_TYPE;




///////////////////////////////////////////////////////////////////////////////
/// CCbException class
/// @author <a href="mailto:peter.b.brunner@siemens.com">Brunner Peter</a> / MED SPD CC
/// @version 1.0
/// @since 28.07.2003
///////////////////////////////////////////////////////////////////////////////

#define _LC_AS_STRING(x)               #x
#define LC_AS_STRING(x)    _LC_AS_STRING(x)
#define __FUNCTION__      __FILE__ ":" LC_AS_STRING(__LINE__) 


/**
 * Begin of a try-error block.
 **/ 
#define SIGFRIED_TRY()                                       \
  try {

/**
 * End of a try-error block.
 **/ 
#define SIGFRIED_CATCH()                                                 \
  } catch (CSIGFRIEDException Exception) {                               \
    throw CSIGFRIEDException(Exception,__FUNCTION__);                    \
  } catch (...) {                                                     \
    throw CSIGFRIEDException(__FUNCTION__,SIGFRIED_EXCEPTION,"SIGFRIED_EXCEPTION"); \
  }                                                                  


#define SIGFRIED_THROW(ErrorCode)                                        \
  throw CSIGFRIEDException(__FUNCTION__,ErrorCode,#ErrorCode);    

#define SIGFRIED_CHECK_MIN_MAX(Value,Min,Max,ErrorCode)                        \
  if (Value < Min || Value > Max) throw CSIGFRIEDException(__FUNCTION__,ErrorCode,#ErrorCode);    

#define SIGFRIED_CHECK(Expression,ErrorCode)                        \
  if (!(Expression)) throw CSIGFRIEDException(__FUNCTION__,ErrorCode,#ErrorCode);    

#define SIGFRIED_CATCH_RETURN()                                          \
  } catch (CSIGFRIEDException Exception) {                               \
    return Exception.m_ErrorCode;                                     \
  } catch (...) {                                                     \
    return SIGFRIED_EXCEPTION;                                           \
  }                                                                   \
  return SIGFRIED_OK;
  
#define SIGFRIED_CATCH_RETURN_STORE(pserror)                             \
  } catch (CSIGFRIEDException Exception) {                               \
    Exception.StoreErrorText(pszerror);                               \
    return Exception.m_ErrorCode;                                     \
  } catch (...) {                                                     \
    return SIGFRIED_EXCEPTION;                                           \
  }                                                                   \
  return SIGFRIED_OK;



class CSIGFRIEDException 
{
public:
	CSIGFRIEDException(const char *szFunctionName, int ErrorCode, const char *szErrorCode);
  CSIGFRIEDException(CSIGFRIEDException LastException, const char *szFunctionName);

  virtual ~CSIGFRIEDException();

  string m_szFunctionNames;
  string m_szErrorCode;

  int    m_ErrorCode;

  void StoreErrorText(CVector<char> *pszerror);



};


#endif // !defined(AFX_CbExcepTION_H__5EB101FC_86AD_45C1_880A_B9514ABF2804__INCLUDED_)

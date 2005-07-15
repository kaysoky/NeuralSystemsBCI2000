////////////////////////////////////////////////////////////////////
// File:    bci_filterwrapper.h
// Date:    Jul 12, 2005
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: A BCI2000 filter wrapper that reads a BCI2000
//          compliant binary stream from an input stream, applies
//          a BCI2000 filter, and writes its output to an
//          output stream as a BCI2000 compliant binary stream.
////////////////////////////////////////////////////////////////////
#include <iostream>
#include "shared/UGenericSignal.h"
#include "shared/UParameter.h"
#include "shared/UState.h"
#include "shared/MessageHandler.h"

class FilterWrapper : public MessageHandler
{
 public:
  FilterWrapper( std::ostream& arOut, std::ostream& arOperator )
  : mrOut( arOut ), mrOperator( arOperator ), mpStatevector( NULL ) {}
  ~FilterWrapper() { delete mpStatevector; }
  void FinishProcessing();

  static const char* FilterName();

 private:
  virtual bool HandlePARAM(       std::istream& );
  virtual bool HandleSTATE(       std::istream& );
  virtual bool HandleVisSignal(   std::istream& );
  virtual bool HandleSTATEVECTOR( std::istream& );

  void StopRun();

 private:
  std::ostream& mrOut;
  std::ostream& mrOperator;
  GenericSignal mOutputSignal;
  PARAMLIST mParamlist;
  STATELIST mStatelist;
  STATEVECTOR* mpStatevector;
};

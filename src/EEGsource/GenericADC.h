/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    GenericADC.CPP                                                  *
 * Comment:   Definition for the GenericADC class                             *
 * Version:   0.04                                                            *
 * Author:    Gerwin Schalk, Juergen Mellinger                                *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 05/11/2000 - First start                                           *
 * V0.02 - 05/26/2000 - changed **RawEEG to *GenericIntSignal                 *
 * V0.03 - 04/11/2002 - included BCI2000ERROR object                          *
 * V0.04 - 03/26/2003 - juergen.mellinger@uni-tuebingen.de:                   *
 *                      derived GenericADC from GenericFilter                 *
 *                      removed GenericADC data members                       *
 ******************************************************************************/
#ifndef GenericADCH
#define GenericADCH

#include "UGenericFilter.h"

class GenericADC : public GenericFilter
{
 protected:
  GenericADC() {}

 public:
  virtual ~GenericADC() {}
  // GenericFilter inherited functions.
  virtual void Preflight( const SignalProperties&,
                                SignalProperties& ) const = 0;
  virtual void Initialize() = 0;
  virtual void Process(   const GenericSignal*,
                                GenericSignal* ) = 0;
  virtual void Halt() = 0;
};

#endif // GenericADCH


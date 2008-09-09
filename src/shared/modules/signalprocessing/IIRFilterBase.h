////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements an IIR filter.
//   Subclasses will provide individual implementations for the
//   DesignFilter() member, which is supposed to translate parameter settings
//   into a rational transfer function (complex poles and zeros).
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef IIR_FILTER_BASE_H
#define IIR_FILTER_BASE_H

#include "GenericFilter.h"
#include "IIRFilter.h"

class IIRFilterBase : public GenericFilter
{
 public:
  typedef double                         Real;
  typedef IIRFilter<Real>::ComplexVector ComplexVector;

 protected:
  IIRFilterBase();
  ~IIRFilterBase();

 public:
  void Preflight( const SignalProperties&, SignalProperties& ) const;
  void Initialize( const SignalProperties&, const SignalProperties& );
  void StartRun();
  void Process( const GenericSignal&, GenericSignal& );

 private:
  // Translate user settings into a filter definition given by
  // - overall gain,
  // - complex roots of the numerator polynomial ("zeros"),
  // - complex roots of the denominator polynomial ("poles").
  virtual void DesignFilter( Real& gain,
                             ComplexVector& zeros,
                             ComplexVector& poles ) const = 0;

  IIRFilter<Real> mFilter;
};

#endif // IIR_FILTER_BASE_H

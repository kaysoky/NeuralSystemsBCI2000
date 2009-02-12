////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The SpatialFilter computes a linear transformation of its
//   input signal, given by a matrix-valued parameter.
//   In this matrix, input channels correspond to columns, and output channels
//   to rows.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPATIAL_FILTER_H
#define SPATIAL_FILTER_H

#include "GenericFilter.h"
#include <valarray>

class SpatialFilter : public GenericFilter
{
 public:
          SpatialFilter();
  virtual ~SpatialFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );

 private:
  template<int SpatialFilterType>
   void DoPreflight( const SignalProperties&, SignalProperties& ) const;
  template<int SpatialFilterType>
   void DoInitialize( const SignalProperties&, const SignalProperties& );
  template<int SpatialFilterType>
   void DoProcess( const GenericSignal& Input, GenericSignal& Output );

  enum
  {
    none,
    fullMatrix,
    sparseMatrix,
    commonAverage
  };
  int mSpatialFilterType;

  typedef float NumType;
  typedef std::valarray<NumType> DataVector;
  std::vector< DataVector > mFilterMatrix;
  DataVector                mSignalBuffer;

  std::vector<int>          mCARoutputList;
};

#endif // SPATIAL_FILTER_H



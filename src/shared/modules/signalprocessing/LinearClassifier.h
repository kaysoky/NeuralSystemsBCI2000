////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The LinearClassifier applies a matrix multiplication to its
//   input data.
//   Input data has 2 indices (N channels x M elements), and output data
//   has a single index (C channels x 1 element), thus the linear classifier
//   acts as a N x M x C matrix, determining the output after summation over
//   N and M.
//
//   The Classifier parameter is a sparse matrix definition in which each row
//   corresponds to a single matrix entry.
//   Columns correspond to
//   1) input channel,
//   2) input element (bin in the spectral case, time offset in the ERP case),
//   3) output channel,
//   4) weight (value of the matrix entry).
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef LINEAR_CLASSIFIER_H
#define LINEAR_CLASSIFIER_H

#include "GenericFilter.h"

class LinearClassifier : public GenericFilter
{
 public:
          LinearClassifier();
  virtual ~LinearClassifier();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );

 private:
  const std::string& DescribeEntry( int row, int col ) const;
  static int Round( double );
  
  std::vector<size_t>  mInputChannels,
                       mInputElements,
                       mOutputChannels;
  std::vector<float>   mWeights;
};

#endif // LINEAR_CLASSIFIER_H



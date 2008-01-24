////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A helper class that represents string labels for indexing
//   matrix and list parameters, and signals.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef LABEL_INDEX_H
#define LABEL_INDEX_H

#include <iostream>
#include <map>
#include <vector>
#include "EncodedString.h"

class LabelIndex
{
  typedef std::map<EncodedString, size_t>  IndexBase;
  typedef std::vector<IndexBase::key_type> IndexReverse;

 public:
  LabelIndex()
    : mNeedSync( false )
    { Reset(); }
  ~LabelIndex() {}

  // Forward lookup.
  IndexBase::mapped_type operator[]( const std::string& ) const;
  // We need this because the above operator must return 0 for nonexisting
  // labels.
  bool Exists( const std::string& ) const;
  // A reverse lookup operator.
  const std::string& operator[]( size_t ) const;
  std::string& operator[]( size_t );

  bool IsTrivial() const;
  static const std::string& TrivialLabel( size_t );

  // Stream I/O.
  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );

  void Resize( size_t );
  int Size() const { return mReverseIndex.size(); }

 private:
  void Sync() const;
  void Reset()
  {
    mReverseIndex.clear();
    mForwardIndex.clear();
    Resize( 1 );
  }

 private:
  // This is the maintained index.
  IndexReverse      mReverseIndex;
  // This is a cache for the more probable lookup direction.
  mutable bool      mNeedSync;
  mutable IndexBase mForwardIndex;
};

inline
std::ostream& operator<<( std::ostream& os, const LabelIndex& l )
{ return l.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, LabelIndex& l )
{ return l.ReadFromStream( is ); }

#endif // LABEL_INDEX_H


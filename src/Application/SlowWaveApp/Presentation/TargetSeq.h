/////////////////////////////////////////////////////////////////////////////
//
// File: TargetSeq.h
//
// Date: Nov 16, 2001
//
// Author: Juergen Mellinger
//
// Description: A list of target codes and associated auditory/visual
//              stimulus files that can read itself from a file.
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TARGETSEQH
#define TARGETSEQH

#include <string>
#include <list>

#include "PresParams.h"
#include "PresErrors.h"

class PARAM;

class TTargetSeqEntry
{
  public:
        TTargetSeqEntry() : targetCode( 0 ) {}
        bool        operator<(  const TTargetSeqEntry &inEntry ) const;

        int         targetCode;
        std::string audioFile;
        std::string visFile;
};


class TTargetSeq: public std::list< TTargetSeqEntry >
{
  public:
    static const int    randomSeqLength = 100; // Length of sequence when created from probabilities.
    TPresError  ReadFromFile(               const char  *inSequenceFileName );
    TPresError  ReadFromParam(              const PARAM *inParamPtr );
    TPresError  CreateFromProbabilities(    const PARAM *inParamPtr );
};

inline
bool
TTargetSeqEntry::operator<( const TTargetSeqEntry   &inEntry ) const
{
    return  TRUE_TARGET_CODE( targetCode ) < TRUE_TARGET_CODE( inEntry.targetCode );
}

#endif // TARGETSEQH



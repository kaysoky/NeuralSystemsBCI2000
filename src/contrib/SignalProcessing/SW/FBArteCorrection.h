////////////////////////////////////////////////////////////////////////////////
//
// File: FBArteCorrection.h
//
// Description: Slow Wave Class Definition
//              written by Dr. Thilo Hinterberger 2000-2001
//              Copyright University of Tuebingen, Germany
//
// Changes:     2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//              Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//              minor reformulations, reformatting.
//              Feb 24, 2004, jm: Moved the TFBArteCorrection class into
//              separate files.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef FBArteCorrectionH
#define FBArteCorrectionH

#include "GenericFilter.h"
#include "GenericVisualization.h"
#include <vector>

class TFBArteCorrection : public GenericFilter
{
  public:
                 TFBArteCorrection();
    virtual      ~TFBArteCorrection() {}
    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize( const SignalProperties&, const SignalProperties& );
    virtual void Process( const GenericSignal&, GenericSignal& );

  private:
    enum ArteMode
    {
      off = 0,
      linearSubtraction,
      subtractionIfSupporting,
      subtractionWithAbort,
    }                    mArteMode;
    std::vector<int>     mArteChList;     // list of input channels assigned with the corresponding ArteCh
    std::vector<float>   mArteFactorList; // correction factor for channel n
};

#endif // FBArteCorrectionH

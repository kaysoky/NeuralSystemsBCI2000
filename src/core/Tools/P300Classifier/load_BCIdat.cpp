#include "load_BCIdat.h"

///////////////////////////////////////////////////////////////////////////////
/// Loads signal, state, and parameter data from the files whose names are given
/// in the path array \e fPathArra.
/// @param [in] fPathArr  Array with the paths of the BCI2000 data files.
/// @param [out] signal   Signal.
/// @param [out] parms    Parameters.
/// @param [out] state    States.
/// \author Cristhian Potes
/// \date July 05, 2009

void load_BCIdat(string FILE, int mode, ap::template_2d_array<float, true>& signal, prm &parms, ste &state)
{
///////////////////////////////////////////////////////////////////////////////
// Section: Define variables
int cum, cc;
vector<int> channelsInFile;
vector<int> samplesInFile;
ap::template_2d_array<float, true> sig;
vector<unsigned short int> stateCode;
vector<unsigned short int> stateType;
vector<unsigned short int> stateSelectedTarget;
vector<unsigned short int> stateSelectedStimulus;
vector<short int> statePhaseInSequence;
vector<unsigned char> stateStimulusBegin;
vector<int> indTarget;
vector<int> SelectedRow;
vector<int> SelectedCol;
vector<unsigned short int> SelectedTarget;
ap::template_1d_array<float, true> SourceChGain;
ap::template_1d_array<float, true> SourceChOffSet;

///////////////////////////////////////////////////////////////////////////////
// Section: Open BCI2000 File, extract parameters, states, and calibrate the signal
BCI2000FileReader* CurrentFile = new BCI2000FileReader;
CurrentFile->Open(FILE.c_str());

int NumSamples = static_cast<int>( CurrentFile->NumSamples() );
int NumChannels = CurrentFile->SignalProperties().Channels();

signal.setbounds(0, NumSamples-1, 0, NumChannels-1);
sig.setbounds(0, NumSamples-1, 0, NumChannels-1);
state.StimulusCode.setbounds(0, NumSamples-1);
state.StimulusType.setbounds(0, NumSamples-1);
state.Flashing.setbounds(0, NumSamples-1);
state.trialnr.setbounds(0, NumSamples-1);
state.TargetDefinitions.clear(); // jm

// Get the signal in float type //
for (int j=0; j<NumChannels; j++)
{
  for (int i=0; i<NumSamples; i++)
    sig(i,j)=static_cast<float>(CurrentFile->RawValue(j,i));
}

//Get some states
const StateVector* stateVector = CurrentFile->StateVector();
for (int i=0; i<NumSamples; i++)
{
  CurrentFile->ReadStateVector(i);
  if (CurrentFile->States()->Exists("StimulusCode"))
    stateCode.push_back(static_cast<unsigned short>(stateVector->StateValue("StimulusCode")));

  if (CurrentFile->States()->Exists("StimulusType"))
    stateType.push_back(static_cast<unsigned short>(stateVector->StateValue("StimulusType")));

  if (CurrentFile->States()->Exists("PhaseInSequence"))
    statePhaseInSequence.push_back(static_cast<short>(stateVector->StateValue("PhaseInSequence")));

  if (CurrentFile->States()->Exists("StimulusBegin"))
    stateStimulusBegin.push_back(static_cast<unsigned char>(stateVector->StateValue("StimulusBegin")));

  if (CurrentFile->States()->Exists("SelectedTarget"))
    stateSelectedTarget.push_back(static_cast<unsigned short>(stateVector->StateValue("SelectedTarget")));

  if (CurrentFile->States()->Exists("SelectedStimulus"))
    stateSelectedStimulus.push_back(static_cast<unsigned short>(stateVector->StateValue("SelectedStimulus")));
}

// Get the channel gains
if (CurrentFile->Parameters()->Exists("SourceChGain"))
{
  const ParamRef parameter = CurrentFile->Parameter("SourceChGain");
  SourceChGain.setbounds(0,parameter->NumValues()-1);
  for (int i=0; i<parameter->NumValues(); i++)
    SourceChGain(i) = (float)CurrentFile->Parameter("SourceChGain")(i);
}

// Get the channel offsets
if (CurrentFile->Parameters()->Exists("SourceChOffSet"))
{
  const ParamRef parameter = CurrentFile->Parameter("SourceChOffSet");
  SourceChOffSet.setbounds(0,parameter->NumValues()-1);
  for (int i=0; i<parameter->NumValues(); i++)
    SourceChOffSet(i) = (float)CurrentFile->Parameter("SourceChOffSet")(i);
}

// Calibrate the signal
for (int j=0; j<NumChannels; j++)
{
  for (int i=0; i<NumSamples; i++)
    signal(i,j) = (sig(i,j) - SourceChOffSet(j))*SourceChGain(j);
}

/////////////////////////////////////////////////////////////////////////////////
// Section: Get parameters

// Get the Sampling Rate
parms.SamplingRate = static_cast<int>(CurrentFile->SamplingRate());

// Get the Number of Sequences
parms.NumberOfSequences = CurrentFile->Parameter("NumberOfSequences");

if (mode == 1 || mode == 2)
{
  // Get the NumMatrixRows
  parms.NumMatrixRows = CurrentFile->Parameter("NumMatrixRows");
  // Get the NumMatrixColumns
  parms.NumMatrixColumns = CurrentFile->Parameter("NumMatrixColumns");
}
else
{
  if (mode == 3 || mode == 4)
  {
    parms.NumMatrixRows = 0;
    parms.NumMatrixColumns = 0;
  }
}

parms.SoftwareCh = NumChannels;

//parms.NumberOfSequences = *(std::min(NumberOfSequences.begin(), NumberOfSequences.end()));


// Get the Target Definitions
if (CurrentFile->Parameters()->Exists("TargetDefinitions"))
{
  const ParamRef parameter = CurrentFile->Parameter("TargetDefinitions");

  for (int i=0; i<parameter->NumRows(); i++)
  {
    enum { Display = 0, Enter = 1 };
    state.TargetDefinitions.push_back( parameter( i, Display ) );
    if( state.TargetDefinitions.back().empty() ) // jm: use the "Enter" value if the "Display" value is empty
      state.TargetDefinitions.back() = string( parameter( i, Enter ) );
  }
}
else
{
  if (mode == 1 || mode == 2)
  {
    printf("The Target Definitions parameters for the P300 Speller task of file %s does not exist", FILE.c_str());
    exit(1);
  }
}

// Get the Stimuli
if (CurrentFile->Parameters()->Exists("Stimuli"))
{
  const ParamRef parameter = CurrentFile->Parameter("Stimuli");

  for (int i=0; i<parameter->NumColumns(); i++) // jm
    state.TargetDefinitions.push_back(parameter(0,i));

}
else
{
  if (mode == 3 || mode == 4)
  {
    printf("The Stimuli parameter for the Stimulus Presentation task of file %s does not exist", FILE.c_str());
    exit(1);
  }
}

// Double check the sizes of the states
if (!(stateCode.size() != stateType.size() != statePhaseInSequence.size() != stateStimulusBegin.size() ))
{
  printf(" Inconsistency in the number of samples of the states. ");
  printf("\n");
  exit(1);
}

// Simulate StimulusType if mode = 1
if (mode == 1)
{
  indTarget.push_back(0); cc = 0;
  for (int i=0; i<NumSamples-1; i++)
  {
    if ((stateSelectedTarget[i] == 0) && (stateSelectedTarget[i+1] > 0))
    {
      SelectedTarget.push_back(stateSelectedTarget[i+1]);
      SelectedRow.push_back(ap::iceil(double (SelectedTarget[cc])/parms.NumMatrixColumns));
      SelectedCol.push_back(SelectedTarget[cc]-parms.NumMatrixColumns*(SelectedRow[cc]-1));
      SelectedCol[cc] = SelectedCol[cc]+parms.NumMatrixColumns;
      indTarget.push_back(i);
      cc++;
    }
  }
  for (size_t i=0; i<indTarget.size()-1; i++)
  {
    for (int j=indTarget[i]; j<indTarget[i+1]; j++)
    {
      if ((stateCode[j] == SelectedRow[i]) || (stateCode[j] == SelectedCol[i]))
        stateType[j] = 1;
      else
        stateType[j] = 0;
    }
  }
}

// Simulate StimulusType if mode = 3
if (mode == 3)
{
  indTarget.push_back(0);
  for (int i=0; i<NumSamples-1; i++)
  {
    if ((stateSelectedStimulus[i] == 0) && (stateSelectedStimulus[i+1] > 0))
    {
      SelectedTarget.push_back(stateSelectedStimulus[i+1]);
      indTarget.push_back(i);
    }
  }
  for (size_t i=0; i<indTarget.size()-1; i++)
  {
    for (int j=indTarget[i]; j<indTarget[i+1]; j++)
    {
      if (stateCode[j] == SelectedTarget[i])
        stateType[j] = 1;
      else
        stateType[j] = 0;
    }
  }
}
for (int i=0; i<NumSamples; i++)
{
  if (stateCode[i] <= parms.NumMatrixRows && stateCode[i] != 0)
    state.StimulusCode(i) = stateCode[i] + parms.NumMatrixColumns;
  else
  {
  if (stateCode[i] > parms.NumMatrixRows && stateCode[i] != 0)
    state.StimulusCode(i) = stateCode[i] - parms.NumMatrixRows;
  else
    state.StimulusCode(i) = 0;
  }

  state.StimulusType(i) = stateType[i];
  state.Flashing(i) = stateStimulusBegin[i];
}
// Create the state trialnr
if (mode == 1 || mode == 2)
{
  cum = 0;
  for (int i=1; i<NumSamples; i++)
  {
    state.trialnr(i-1) = cum;
    if ((statePhaseInSequence[i-1] != 2) && (statePhaseInSequence[i] == 2))
      cum++;
  }
}

// Create the state trialnr
if (mode == 3 || mode == 4)
{
#if 0 // jm
  cum = 1;
  indTarget.clear();
  indTarget.push_back(0);
  for (int i=0; i<NumSamples-1; i++)
  {
    if ((stateSelectedStimulus[i] > 0) && (stateSelectedStimulus[i+1] == 0))
      indTarget.push_back(i);
  }
  indTarget.push_back(NumSamples-1);
  for (size_t i=0; i<indTarget.size()-1; i++)
  {
    for (int j=indTarget[i]; j<indTarget[i+1]; j++)
      state.trialnr(j) = cum;
    cum++;
  }
}
#else // jm
  if( CurrentFile->Parameters()->Exists( "SequenceType" ) && CurrentFile->Parameter( "SequenceType" ) == 2 ) // P3Speller compatible
  {
    int trial = 0;
    for( int i = 1; i < NumSamples; ++i )
    {
      state.trialnr( i - 1 ) = trial;
      if( ( statePhaseInSequence[i - 1] != 2 ) && ( statePhaseInSequence[i] == 2 ) )
        ++trial;
    }
  }
  else
  {
    int trial = 1,
        sequence = 0;

    for( int i = 1; i < NumSamples; ++i )
    {
      state.trialnr( i - 1 ) = trial;
      if( ( statePhaseInSequence[ i - 1 ] != 2 ) && ( statePhaseInSequence[i] == 2 ) )
      {
        if( ++sequence == parms.NumberOfSequences + 1 )
        {
          sequence = 1;
          ++trial;
        }
      }
    }
  }
#endif // jm
}

delete CurrentFile;
}

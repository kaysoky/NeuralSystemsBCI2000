#include "Check_Consistency_Files.h"
#include <stdlib.h>
///////////////////////////////////////////////////////////////////
/// Check if the provided files are consistent in terms of their
/// parameters and states
/// @param [in]		fPathArr		Array containing all the paths of the files 
/// @param [out]	numSamples		Total number of samples 
/// @param [out]	numChannels		Total number of channels  
/// \author Cristhian Potes
/// \date July 12, 2009

int Check_Consistency_Files(vector<string> fPathArr, long long &numSamples, int &numChannels)
{
///////////////////////////////////////////////////////////////////////////////
// Section: Define variables
size_t files;
int i, mode;
numSamples = 0;
vector<int> channelsInFile;
vector<long long> samplesInFile;
vector<int> SamplingRate;
vector<int> NumberOfSequences;
vector<int> NumMatrixRows;
vector<int> NumMatrixColumns;
vector<int> NumStimuli;
vector<int> InterpretMode;
vector<string> ApplicationFilterChain;
vector<string> Experiment;

///////////////////////////////////////////////////////////////////////////////
// Section: Open BCI2000 File, check if the file is open, and check compatibility 
// among the files
BCI2000FileReader* CurrentFile = new BCI2000FileReader; 
Experiment.push_back("P3SpellerTask");
Experiment.push_back("StimulusPresentationTask");
// Check if the BCI2000 file exists
for (files=0; files<fPathArr.size(); files++)
{
	CurrentFile->Open(fPathArr[files].c_str()); 
	if( !CurrentFile->IsOpen() )
        CurrentFile->Open( ( fPathArr[files] + ".dat" ).c_str() );
	if( !CurrentFile->IsOpen() )
      {
        ostringstream oss;
        oss << "Could not open \"" << fPathArr[files].c_str() << "\" as a BCI2000 data file.";
        printf( oss.str().c_str() );
		exit(1);
      }
	// Check whether files are compatible.
	const StateList* statelist = CurrentFile->States();
	for (i=0; i<statelist->Size(); i++ )
        if( !CurrentFile->States()->Exists((*statelist)[i].Name()))
		{
          printf( "Incompatible state information across input files." );
		  printf("\n");
		  exit(1);
		}

	channelsInFile.push_back(CurrentFile->SignalProperties().Channels());
	samplesInFile.push_back(CurrentFile->NumSamples());
	
	// Check the Interpret Mode
	if (CurrentFile->Parameters()->Exists("InterpretMode"))
	{
		InterpretMode.push_back(CurrentFile->Parameter("InterpretMode"));
		if (*InterpretMode.rbegin() == 0)
		{
			printf("Mode of data file %s is not supported\n", fPathArr[files].c_str());
			exit(1);
		}
	}
	else
	{
		printf("Interpret mode of file %s does not exist", fPathArr[files].c_str());
		exit(1);
	}

	// Check the Application Filter Chain
	if (CurrentFile->Parameters()->Exists("ApplicationFilterChain"))
		ApplicationFilterChain.push_back(CurrentFile->Parameter("ApplicationFilterChain")(1));
	else
	{
		printf("Application Filter Chain of file %s does not exist", fPathArr[files].c_str());
		exit(1);
	}

	// Check the Sampling Rate
	if (CurrentFile->Parameters()->Exists("SamplingRate"))
    SamplingRate.push_back(static_cast<int>(CurrentFile->SamplingRate()));
	else
	{
		printf("The sampling rate of file %s does not exist", fPathArr[files].c_str());
		exit(1);
	}

	// Check the NumMatrixRows
	if (CurrentFile->Parameters()->Exists("NumMatrixRows"))
		NumMatrixRows.push_back(CurrentFile->Parameter("NumMatrixRows"));
	else
	{
		if (ApplicationFilterChain[files] == Experiment[0])
		{
			printf("The number of rows for the P300 Speller paradigm of file %s does not exist", fPathArr[files].c_str());
			exit(1);
		}
	}

	// Check the NumMatrixColumns
	if (CurrentFile->Parameters()->Exists("NumMatrixColumns"))
		NumMatrixColumns.push_back(CurrentFile->Parameter("NumMatrixColumns"));
	else
	{
		if (ApplicationFilterChain[files] == Experiment[0])
		{
			printf("The number of columns for the P300 Speller paradigm of file %s does not exist", fPathArr[files].c_str());
			exit(1);
		}
	}

	// Check the number of stimulus in the Stimulus Presentation Task
	if (CurrentFile->Parameters()->Exists("Stimuli"))
	{
		const ParamRef parameter = CurrentFile->Parameter("Stimuli");
		NumStimuli.push_back(parameter->NumValues());
	}
	else
	{
		if (ApplicationFilterChain[files] == Experiment[1])
		{
			printf("There is no stimuli for the Stimulus Presentation Task of file %s", fPathArr[files].c_str());
			exit(1);
		}
	}


	if (files>0)
	{
		if (channelsInFile[files-1] != channelsInFile[files])
		{	
			BCIError::ConfigurationError("All input files must have the same numbers of channels.");
			exit(1);
		}
		if (SamplingRate[files-1] != SamplingRate[files])
		{	
			BCIError::ConfigurationError("All input files must have the same sampling rate.");
			exit(1);
		}
		if (InterpretMode[files-1] != InterpretMode[files])
		{	
			BCIError::ConfigurationError("All input files must have the same Interpret Mode.");
			exit(1);
		}
		if (ApplicationFilterChain[files-1] != ApplicationFilterChain[files])
		{	
			BCIError::ConfigurationError("All input files must have been recorded from the same experiment.");
			exit(1);
		}
		if (ApplicationFilterChain[files] == Experiment[0])
			if (NumMatrixRows[files-1] != NumMatrixRows[files])
			{	
				BCIError::ConfigurationError("All input files must be recorded under the same paradigm.");
				exit(1);
			}
		if (ApplicationFilterChain[files] == Experiment[0])
			if (NumMatrixColumns[files-1] != NumMatrixColumns[files])
			{	
				BCIError::ConfigurationError("All input files must be recorded under the same paradigm.");
				exit(1);
			}
		if (ApplicationFilterChain[files] == Experiment[1])
			if (NumStimuli[files-1] != NumStimuli[files])
			{	
				BCIError::ConfigurationError("All input files must have the same number of stimulus.");
				exit(1);
			}

	}
	numSamples += samplesInFile[files];
}
numChannels = channelsInFile[0];
// Set mode according to the Interpret Mode and Application Filter Chain parameters
// mode = 1		->		P3SpellerTask/Online Free Mode
// mode = 2		->		P3SpellerTask/Copy Mode
// mode = 3		->		StimulusPresentationTask/Online Free Mode
// mode = 4		->		StimulusPresentationTask/Copy Mode

if ((Experiment[0] == ApplicationFilterChain[0]) && InterpretMode[0] == 1)
	mode = 1;
if ((Experiment[0] == ApplicationFilterChain[0]) && InterpretMode[0] == 2)
	mode = 2;
if ((Experiment[1] == ApplicationFilterChain[0]) && InterpretMode[0] == 1)
	mode = 3;
if ((Experiment[1] == ApplicationFilterChain[0]) && InterpretMode[0] == 2)
	mode = 4;
CurrentFile->~BCI2000FileReader();
return mode;
}

#include "P300_Classify.h"

///////////////////////////////////////////////////////////////////
/// Predict the intended letters according to the pscore matrix.
/// This function assumes that the data was recorded under the P300
/// Speller paradigm.  
/// @param [in] pscore				Matrix containing the score values for each row and column
/// @param [in] Code				ste Stimulus Code
/// @param [in] Type				ste Stimulus Type
/// @param [in] trialnr				ste trialnr
///	@param [in] NumberOfSequences	Number of sequences. Default = 15;
///	@param [in] NumberMatrixRows	Number of rows for the P300 Speller
///	@param [in] NumberMatrixColumns	Number of columns for the P300 Speller
///	@param [in] TargetDefinitions	Vector containing the characters from the P300 Speller									
///	@param [out] result				Vector containing the performances. The values are given in percentages
///	@param [out] predicted			Vecto containing the predicted character
/// \author Cristhian Potes
/// \date June 29, 2009

void P300_Classify(ap::real_2d_array pscore, 
				   ap::template_1d_array<unsigned short int, true>& StimulusCode,
				   ap::template_1d_array<unsigned short int, true>& StimulusType,
				   ap::template_1d_array<short int, true>& trialnr,
				   int NumberOfSequences,
				   int NumMatrixRows,
				   int NumMatrixColumns,
				   vector<string> TargetDefinitions,
				   vector<double>& result,
				   vector<string>& predicted)
{

////////////////////////////////////////////////////////////////////////
// Section: Define variables
int choice, epoch;
double val, max_value_row= -1e15, max_value_col= -1e15, numletters, correct;
ap::real_2d_array cflash;
ap::real_2d_array score;
ap::template_2d_array<int, true> predictedcol;
ap::template_2d_array<int, true> predictedrow;
vector<int> range;
vector<int> codes;
vector<int> codecol;
vector<int> coderow;
vector<int>::iterator it;

numletters = (pscore.gethighbound(0)+1)/NumberOfSequences;
choice = NumMatrixRows + NumMatrixColumns;
epoch = NumberOfSequences * choice;
cflash.setbounds(0, choice-1, 0, pscore.gethighbound(0));
score.setbounds(0, static_cast<int>(numletters-1), 0, choice-1);
predictedrow.setbounds(0, static_cast<int>(numletters-1), 0, NumberOfSequences-1);
predictedcol.setbounds(0, static_cast<int>(numletters-1), 0, NumberOfSequences-1);

////////////////////////////////////////////////////////////////////////
// Predict letters according to pscore
for (int i=0; i<numletters; i++)
{
	for (int j=0; j<trialnr.gethighbound(1)+1; j++)
	{
		if (trialnr(j) == i+1)
			range.push_back(j);
	}
	if (range.size() != 0)
	{
		for (size_t j=0; j<range.size(); j++)
			codes.push_back(StimulusCode(range[j])*StimulusType(range[j]));

		sort(codes.begin(), codes.end());
		it = unique(codes.begin(), codes.end()); 
		codes.resize(it-codes.begin());

		if (codes.size() >= 2)
			codecol.push_back(codes[1]);
		if (codes.size() > 2)
			coderow.push_back(codes[2]);
		
		codes.clear();
		range.clear();

		for (int j=0; j<choice; j++)
		{
			val = 0;
			for (int k=i*NumberOfSequences; k<(i+1)*NumberOfSequences; k++)
			{
				val += pscore(j,k);
				cflash(j,k) = val;
			}
			score(i,j) = val;
		}		
	}
}
for (int i=0; i<numletters; i++)
{
	int l = 0;
	for (int k=i*NumberOfSequences; k<(i+1)*NumberOfSequences; k++)
	{
		for (int j=0; j<choice; j++)
		{
			if ((j<NumMatrixColumns))
			{
				/* compare with max */
				if (cflash(j,k) > max_value_col)
				{
					max_value_col = cflash(j,k);
					predictedcol(i,l) = j + 1;
				}
			}
			if ((j>=NumMatrixColumns))
			{
				/* compare with max */
				if (cflash(j,k) > max_value_row)
				{
					max_value_row = cflash(j,k);
					predictedrow(i,l) = j + 1;
				}
			}
		}
		l++, max_value_col= -1e15, max_value_row= -1e15;
	}
}

for (int j=0; j<NumberOfSequences; j++)
{
	correct= 0;
	for (int i=0; i<numletters; i++)
	{
		if ((coderow[i] == predictedrow(i,j)) && codecol[i] == predictedcol(i,j))
		{
			predicted.push_back(TargetDefinitions[(NumMatrixColumns*(predictedrow(i,j)-1-NumMatrixColumns))+predictedcol(i,j)-1]);
			correct++;
		}
		else
			predicted.push_back(TargetDefinitions[(NumMatrixColumns*(predictedrow(i,j)-1-NumMatrixColumns))+predictedcol(i,j)-1]);
	}
	result.push_back((correct/numletters)*100);
}
}

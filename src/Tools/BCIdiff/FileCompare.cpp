#pragma hdrstop
#include "FileCompare.h"
#include "FileReader.h"
#include "UParameter.h"
#include "UState.h"

#include<iostream>
#include <set>

using namespace std;

FileCompare::FileCompare()
{
}

FileCompare::~FileCompare()
{
}

void FileCompare::setFiles(FileReader* file1, FileReader* file2)
{
        mFile1=file1;
        mFile2=file2;
}


bool FileCompare::compareHeaderLengths()
{
        if(mFile1->getHeaderLength()==mFile2->getHeaderLength())
                return true;
        else
                return false;
}

bool FileCompare::compareStateVectorLengths()
{
        if(mFile1->getStateVectorLength()==mFile2->getStateVectorLength())
                return true;
        else
                return false;
}

bool FileCompare::compareNumChannels()
{
        if(mFile1->getNumChannels()==mFile2->getNumChannels())
                return true;
        else
                return false;
}

bool FileCompare::compareSampleFrequencies()
{
        if(mFile1->getSampleFrequency()==mFile2->getSampleFrequency())
                return true;
        else
                return false;
}

bool FileCompare::compareNumSamples()
{
        if(mFile1->getNumSamples()==mFile2->getNumSamples())
                return true;
        else
                return false;
}

bool FileCompare::compareParams()
{
        bool noDifferences=true;
        set<string> paramLabels;
        const PARAMLIST* para1=mFile1->getParamListPtr();

        for(int i=0;i<(int)(para1->GetNumParameters());++i)
             paramLabels.insert(para1->GetParamPtr(i)->GetName());

        const PARAMLIST* para2=mFile2->getParamListPtr();
        for(int i=0;i<(int)(para2->GetNumParameters());++i)
                paramLabels.insert(para2->GetParamPtr(i)->GetName());

        for(set<string>::const_iterator i=paramLabels.begin();i!=paramLabels.end();++i)
        {
                if((para1->GetParamPtr(i->c_str())!=NULL)&&(para2->GetParamPtr(i->c_str())!=NULL))
                {
                        if((string)(para1->GetParamPtr(i->c_str())->GetValue())!=para2->GetParamPtr(i->c_str())->GetValue())
                        {
                                cerr<<"Parameter "<<*i<<" differs."<<'\n';
                                cerr<<*i<<" in "<<mFile1->getFileName()<<" has value "<< para1->GetParamPtr(i->c_str())->GetValue()<<".\n";
                                cerr<<*i<<" in "<<mFile2->getFileName()<<" has value "<< para2->GetParamPtr(i->c_str())->GetValue()<<".\n";
                                cerr<<'\n';
                                noDifferences=false;
                        }
                }
                else
                {
                        cerr<<"Parameter "<<*i<<"does not exist in both files."<<'\n';
                        cerr<<'\n';
                        noDifferences=false;
                }
        }

        return noDifferences;
}

bool FileCompare::compareStates()
{
        bool noDifferences=true;

        const STATELIST* stateL1=mFile1->getStateListPtr();
        const STATELIST* stateL2=mFile2->getStateListPtr();
        const STATEVECTOR* stateV1=mFile1->getStateVectorPtr();
        const STATEVECTOR* stateV2=mFile2->getStateVectorPtr();

        if(stateL1->GetNumStates()!=stateL2->GetNumStates())
        {
                cerr<<"Number of states differs. \n";
                return false;
        }

        set<string> stateLabels;

        for(int i=0; i<stateL1->GetNumStates(); ++i)
                stateLabels.insert(stateL1->GetStatePtr(i)->GetName());

        for(int i=0; i<stateL2->GetNumStates(); ++i)
                stateLabels.insert(stateL2->GetStatePtr(i)->GetName());

        for(set<string>::const_iterator i=stateLabels.begin();i!=stateLabels.end();++i)
        {
                if(stateV1->GetStateValue(i->c_str())!=stateV2->GetStateValue(i->c_str()))
                {
                        cerr<<"State "<<*i<<" has value " << stateV1->GetStateValue(i->c_str())
                                <<" in "<<mFile1->getFileName()<<", whereas "<<*i<<" has value "
                                << stateV2->GetStateValue(i->c_str()) <<" in "<<mFile2->getFileName()<<". \n";
                        noDifferences=false;
                }
        }
        cerr<<'\n';
        return noDifferences;
}

bool FileCompare::compareValues()
{
        bool noDifferences=true;
        int num_vals_same=0;
        unsigned long max_num_samples=min(mFile1->getNumSamples(),mFile2->getNumSamples());
        int max_num_channels=min(mFile1->getNumChannels(),mFile2->getNumChannels());

        for(unsigned long mom_sample=0;mom_sample<max_num_samples; ++mom_sample)
        {
                for(int mom_channel=0; mom_channel<max_num_channels; ++mom_channel)
                {
                        if(mFile1->readValue(mom_channel, mom_sample)==mFile2->readValue(mom_channel, mom_sample))
                        {       /*
                                mFile1->readStateVector(mom_sample);
                                mFile2->readStateVector(mom_sample);
                                if(!compareStates())
                                        noDifferences=false;
                                else   */
                                        ++num_vals_same;
                        }
                        else
                        {
                                cerr<<"Sample "<<mom_sample<<" on channel "<<mom_channel<<" has value "
                                        <<mFile1->readValue(mom_channel, mom_sample)<<" in file one and value "
                                        <<mFile2->readValue(mom_channel, mom_sample)<<" in file two. \n";
                                noDifferences=false;
                        }
                }
        }

        cerr<<"The files "<<mFile1->getFileName()<<" and "<<mFile2->getFileName()<<" had "
                <<num_vals_same<<" of "<<max_num_samples*max_num_channels<<" compared sample values that were the same. \n";
        cerr<<'\n';
        return noDifferences;
}



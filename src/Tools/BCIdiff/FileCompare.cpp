#pragma hdrstop
#include "FileCompare.h"
#include "FileReader.h"
#include "UParameter.h"
#include "UState.h"

#include<iostream>
#include<set>
#include<vector>

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


bool FileCompare::headerLengthsDiffer()
{
        if(mFile1->getHeaderLength()==mFile2->getHeaderLength())
                return false;
        else
        {
                cout<<"Headerlength is "<<mFile1->getHeaderLength()<<" in file "
                        <<mFile1->getFileName()<<" and "<<mFile2->getHeaderLength()
                        <<" in file "<<mFile2->getFileName()<<'\n';
                return true;
        }
}

bool FileCompare::stateVectorLengthsDiffer()
{
        if(mFile1->getStateVectorLength()==mFile2->getStateVectorLength())
                return false;
        else
        {
                cout<<"Statevectorlength is "<<mFile1->getStateVectorLength()<<" in file "
                        <<mFile1->getFileName()<<" and "<<mFile2->getStateVectorLength()
                        <<" in file "<<mFile2->getFileName()<<'\n';
                return true;
        }
}

bool FileCompare::numChannelsDiffer()
{
        if(mFile1->getNumChannels()==mFile2->getNumChannels())
                return false;
        else
        {
                cout<<"Number of channels is "<<mFile1->getNumChannels()<<" in file "
                        <<mFile1->getFileName()<<" and "<<mFile2->getNumChannels()
                        <<" in file "<<mFile2->getFileName()<<'\n';
                return true;
        }
}

bool FileCompare::sampleFrequenciesDiffer()
{
        if(mFile1->getSampleFrequency()==mFile2->getSampleFrequency())
                return false;
        else
        {
                cout<<"Samplefrequency is "<<mFile1->getSampleFrequency()<<" in file "
                        <<mFile1->getFileName()<<" and "<<mFile2->getSampleFrequency()
                        <<" in file "<<mFile2->getFileName()<<'\n';
                return true;
        }
}

bool FileCompare::numSamplesDiffer()
{
        if(mFile1->getNumSamples()==mFile2->getNumSamples())
                return false;
        else
        {
                cout<<"Number of samples is "<<mFile1->getNumSamples()<<" in file "
                        <<mFile1->getFileName()<<" and "<<mFile2->getNumSamples()
                        <<" in file "<<mFile2->getFileName()<<'\n';
                return true;
        }
}

bool FileCompare::paramsDiffer()
{
        bool differ=false;
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
                                cout<<"Parameter "<<*i<<" differs."<<'\n';
                                cout<<*i<<" in "<<mFile1->getFileName()<<" has value "<< para1->GetParamPtr(i->c_str())->GetValue()<<".\n";
                                cout<<*i<<" in "<<mFile2->getFileName()<<" has value "<< para2->GetParamPtr(i->c_str())->GetValue()<<".\n";
                                cout<<'\n';
                                differ=true;
                        }
                }
                else
                {
                        cout<<"Parameter "<<*i<<"does not exist in both files."<<'\n';
                        cout<<'\n';
                        differ=true;
                }
        }

        return differ;
}

bool FileCompare::stateListsDiffer()
{
        bool differ=false;

        const STATELIST* stateL1=mFile1->getStateListPtr();
        const STATELIST* stateL2=mFile2->getStateListPtr();

        if(stateL1->GetNumStates()!=stateL2->GetNumStates())
        {
                cout<<"Number of states differs. \n";
                return true;
        }

        for(int i=0; i<stateL1->GetNumStates();i++)
        {
                if(string(stateL1->GetStatePtr(i)->GetName())!=string(stateL2->GetStatePtr(i)->GetName()))
                {
                        cout<<"State "<<i<<" has name "<<stateL1->GetStatePtr(i)->GetName()
                                <<" in file "<<mFile1->getFileName()<<" and name "
                                <<stateL2->GetStatePtr(i)->GetName()<<" in file "
                                <<mFile2->getFileName()<<'\n';
                        differ=true;
                }
        }
        return differ;
}


bool FileCompare::currentStatesDiffer()
{
        bool differ=false;

        const STATELIST* stateL1=mFile1->getStateListPtr();
        const STATEVECTOR* stateV1=mFile1->getStateVectorPtr();
        const STATEVECTOR* stateV2=mFile2->getStateVectorPtr();

        for(int i=0;i<stateL1->GetNumStates();++i)
        {
                if(stateV1->GetStateValue(stateL1->GetStatePtr(i)->GetName())!=stateV2->GetStateValue(stateL1->GetStatePtr(i)->GetName()))
                {
                        cout<<"State "<<stateL1->GetStatePtr(i)->GetName()<<" has value " << stateV1->GetStateValue(stateL1->GetStatePtr(i)->GetName())
                                <<" in "<<mFile1->getFileName()<<", whereas "<<stateL1->GetStatePtr(i)->GetName()<<" has value "
                                << stateV2->GetStateValue(stateL1->GetStatePtr(i)->GetName()) <<" in "<<mFile2->getFileName()<<". \n";
                        differ=true;
                }
        }
        cout<<'\n';
        return differ;
}

bool FileCompare::valuesDiffer(bool compareStates)
{
        bool differ=false;
        int num_vals_same=0;
        unsigned long max_num_samples=min(mFile1->getNumSamples(),mFile2->getNumSamples());
        int max_num_channels=min(mFile1->getNumChannels(),mFile2->getNumChannels());

        for(unsigned long mom_sample=0;mom_sample<max_num_samples; ++mom_sample)
        {
                for(int mom_channel=0; mom_channel<max_num_channels; ++mom_channel)
                {
                        if(mFile1->readValue(mom_channel, mom_sample)==mFile2->readValue(mom_channel, mom_sample))
                        {
                                if(compareStates)
                                {
                                        mFile1->readStateVector(mom_sample);
                                        mFile2->readStateVector(mom_sample);
                                        if(!currentStatesDiffer())
                                                differ=true;
                                        else
                                                ++num_vals_same;
                                }
                                else
                                        ++num_vals_same;
                        }
                        else
                        {
                                cout<<"Sample "<<mom_sample<<" on channel "<<mom_channel<<" has value "
                                        <<mFile1->readValue(mom_channel, mom_sample)<<" in file one and value "
                                        <<mFile2->readValue(mom_channel, mom_sample)<<" in file two. \n";
                                differ=true;
                        }
                }
        }

        cout<<"The files "<<mFile1->getFileName()<<" and "<<mFile2->getFileName()<<" had "
                <<num_vals_same<<" of "<<max_num_samples*max_num_channels<<" compared sample values that were the same. \n";
        cout<<'\n';
        return differ;
}



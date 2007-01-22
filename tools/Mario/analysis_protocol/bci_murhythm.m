function VirtualStates = bci_murhythm(States, varargin)
% BCI_MURHYTHM implement analyasis protocol for EEG rhythm modulation
% TODO: help, licence
%
% Authors: A. Emanuele Fiorilla

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

% TODO:  fix virtual states datatype
%        check output state names to follow standard
TargetsToEvaluate = varargin{1};

VectorStateLength = length(States.TargetCode);

VirtualStates.ValidSamples=zeros(1,VectorStateLength);
VirtualStates.Regressor=zeros(1,VectorStateLength);

NumberOfTargets=length(TargetsToEvaluate);

for target=1:NumberOfTargets
    VirtualStates.ValidSamples(find(States.TargetCode==TargetsToEvaluate(target)))=1;
end
VirtualStates.ValidSamples = logical(VirtualStates.ValidSamples);       %Casting to Logical
VirtualStates.Regressor = States.TargetCode';
VirtualStates.Regressor(~VirtualStates.ValidSamples) = 0;

StartTrials = [1;find(diff(States.IntertrialInterval)<0)+1]; %ITI is the end of trial
StartTrials_end =[StartTrials;VectorStateLength];
for TrialNumerator = 1:length(StartTrials_end)-1
    VirtualStates.TrialNum(StartTrials_end(TrialNumerator):StartTrials_end(TrialNumerator+1))=TrialNumerator;
    VirtualStates.TrialTime(StartTrials_end(TrialNumerator):StartTrials_end(TrialNumerator+1))=1:1:(StartTrials_end(TrialNumerator+1)-StartTrials_end(TrialNumerator)+1);
    VirtualStates.TrialClass(StartTrials_end(TrialNumerator):StartTrials_end(TrialNumerator+1))=VirtualStates.Regressor(StartTrials_end(TrialNumerator));
end
VirtualStates.TrialStart = int16([0 diff(VirtualStates.TrialNum)]);
VirtualStates.Break = zeros(size(VirtualStates.Regressor));
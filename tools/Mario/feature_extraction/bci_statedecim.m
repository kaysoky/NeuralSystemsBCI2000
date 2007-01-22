function EpochStates = bci_statextractor(FEMethod, States, EpochIndex)
% BCI_STATDECIM reduce status waveforms to one sample per epoch (feat. vector) 
% TODO: help, licence

% Author: A. Emanuele Fiorilla.

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

StateNames = fieldnames(States);
VectorStateLength = length(States.(StateNames{1}));

switch lower(FEMethod)
   case 'iterativemem'
      for EpochNum = 1:size(EpochIndex,2)
         if (length(unique(States.Regressor(EpochIndex(1,EpochNum):EpochIndex(2,EpochNum)))) == 1) & (unique(States.Regressor(EpochIndex(1,EpochNum):EpochIndex(2,EpochNum)))~=0)
            EpochStates.Regressor(EpochNum) = unique(States.Regressor(EpochIndex(1,EpochNum):EpochIndex(2,EpochNum)));
         end
      end
   case 'p300_fe'
      TrialStart_end=[States.TrialStart;VectorStateLength+1];
      for TrialIndex = 1:length(TrialStart_end)-1
         TimeDomainTrialNum(TrialStart_end(TrialIndex):TrialStart_end(TrialIndex+1)-1) =  TrialIndex;
      end% for
      for EpochNum = 1:size(EpochIndex,2)
         EpochStates.Regressor(EpochNum) = States.Regressor(EpochIndex(1,EpochNum));
         EpochStates.TrialNum(EpochNum) = TimeDomainTrialNum(EpochIndex(1,EpochNum));
         EpochStates.StimulusCode(EpochNum) = States.StimulusCode(EpochIndex(1,EpochNum));
      end% for
end% switch
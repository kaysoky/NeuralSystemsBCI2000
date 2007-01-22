function VirtualStates = BCI_P300(States, varargin)

VectorStateLength = length (States.Flashing);

StartTrials = find((States.Flashing(1:VectorStateLength-1)==0)&(States.Flashing(2:VectorStateLength)==1))+1;
StartTrials_end =[StartTrials;VectorStateLength];

for TrialNumerator = 1:length(StartTrials_end)-1
    TrialNum(StartTrials_end(TrialNumerator):StartTrials_end(TrialNumerator+1))=TrialNumerator;
end
VirtualStates.Regressor = int16(States.StimulusType'+1);
VirtualStates.ValidSamples = true(size(VirtualStates.Regressor));
VirtualStates.InTrialTrigger = int16([0 diff(TrialNum)]);
VirtualStates.TrialStart = find(States.PhaseInSequence(1:end-1)~=1 & States.PhaseInSequence(2:end)==1);
VirtualStates.Break = false(size(VirtualStates.Regressor));
VirtualStates.StimulusCode = States.StimulusCode;

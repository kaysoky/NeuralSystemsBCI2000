function Epoch_matr = P300_Epocher(States,SampEpochLength,EpochOverlap)

InTrialTrigger = States.InTrialTrigger;
ValidSamples = States.ValidSamples;
Break = States.Break;

i = 1;

start_Samps = find(InTrialTrigger&ValidSamples);

for jj = 1:length(start_Samps)
    epoch = [start_Samps(jj):start_Samps(jj)+ SampEpochLength];
    if ((~isempty(find(ValidSamples(epoch)))) & (length(find(ValidSamples(epoch)))==length(epoch))) &...
            (~isempty(find(InTrialTrigger(epoch))) | find(find(InTrialTrigger(epoch))==1)) &...
                (isempty(find(Break(epoch))))
        Epoch_matr(:,i) = [start_Samps(jj) start_Samps(jj)+ SampEpochLength];
        i = i+1;
    end
end
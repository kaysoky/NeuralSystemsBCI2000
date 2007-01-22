function Epoch_matr = bci_muepocher(States, SampEpochLength, EpochOverlap)
% BCI_MUEPOCHER

TrialStart = States.TrialStart;
ValidSamples = States.ValidSamples;

i = 1;
start_Samp = 1;
end_Samp = SampEpochLength;
SampShift = round(SampEpochLength-EpochOverlap*SampEpochLength);

while end_Samp<=length(ValidSamples)
    epoch = [start_Samp:end_Samp];
    if ((~isempty(find(ValidSamples(epoch)))) & (length(find(ValidSamples(epoch)))==length(epoch))) &...
            (isempty(find(TrialStart(epoch))) | ((length(find(TrialStart(epoch))) == 1) & (find(TrialStart(epoch)) == 1)))
        Epoch_matr(:,i) = [start_Samp;end_Samp];
        start_Samp_prev = start_Samp;
        start_Samp = start_Samp_prev + SampShift;
        end_Samp = start_Samp + SampEpochLength - 1;
        i = i+1;
    else
        start_Samp = start_Samp + 1;
        end_Samp = start_Samp + SampEpochLength - 1;
    end% if
end% while
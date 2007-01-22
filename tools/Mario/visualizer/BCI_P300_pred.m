function Predicted = BCI_P300_pred(FeatExtraction,DAQParams,FeatMatrix)

TargetMatrix = DAQParams.TargetDefinitionMatrix.Value;

TrialNumbers = unique(FeatExtraction.States.TrialNum);
NumberOfFeatures = size(FeatMatrix,1);

for TrialIndex = 1:length(TrialNumbers)
    TrialSamples = find(FeatExtraction.States.TrialNum == TrialNumbers(TrialIndex));
    for FeatureIndex = 1:NumberOfFeatures
        LatencySamp = FeatMatrix(FeatureIndex,1);
        ChannelNum = FeatMatrix(FeatureIndex,2);
        FeatureWeight = FeatMatrix(FeatureIndex,3);
        SingleTrialData(FeatureIndex,:) = FeatureWeight .* FeatExtraction.Data(LatencySamp,ChannelNum,TrialSamples);
    end
    StimulusCode = FeatExtraction.States.StimulusCode;
    Stimuli = unique(StimulusCode);
    for StimulusIndex = 1:length(Stimuli)
        TrialControlSig = sum(SingleTrialData,1);
        MeanValue(StimulusIndex) = mean(TrialControlSig(find(StimulusCode(TrialSamples)==Stimuli(StimulusIndex))));
    end
    ColMeans = MeanValue([1:str2num(DAQParams.NumMatrixColumns.Value{1})]);
    RowMeans = MeanValue(str2num(DAQParams.NumMatrixColumns.Value{1}) + [1:str2num(DAQParams.NumMatrixRows.Value{1})]);
    TargetCol = find(ColMeans == max(ColMeans));
    TargetRow = find(RowMeans == max(RowMeans));
    TargetMatrix = reshape(cell2mat(DAQParams.TargetDefinitionMatrix.Value(:,1)),[str2num(DAQParams.NumMatrixRows.Value{1}) str2num(DAQParams.NumMatrixColumns.Value{1})]);
    Predicted(TrialIndex)=TargetMatrix(TargetCol,TargetRow);
end
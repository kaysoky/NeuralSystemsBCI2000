function FeatMatr = P300_FE(Data,FExParams,EpochIndex)

FEParmsVect = [FExParams.EpochOverlap FExParams.EpochLength FExParams.FSamp];
for EpochNum = 1:size(EpochIndex,2)
    Features = Data(EpochIndex(1,EpochNum):EpochIndex(2,EpochNum),:);
    FeatMatr(:,:,EpochNum)=Features;
end

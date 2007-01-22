Data = BCI2k.FeatExtraction.Data;
PackedFeatures=reshape(Data,[size(Data,1)*size(Data,2) size(Data,3)])';
Regressor = double(BCI2k.FeatExtraction.States.Regressor)';
penter = 0.1;
premove = 0.15;
[B,SE,PVAL,in] = stepwisefit(PackedFeatures,Regressor,'display','off','penter',penter,'premove',premove);% 'penter',.1,'premove',.15
function hfig = TopographViewer(BCI2k,x,y)
load('AvgSubj_skin.mat');
posFullFileName = 'AvgSubj_ATT.dig';
DataVect = BCI2k.Statistics.Data(x,:)';
DoMapOnScalp (skin,posFullFileName,DataVect)
axis ij;
view(2);
hfig = 0;
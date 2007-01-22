tic
clear
clc

% Dat_Files={'E:\Documentazione Tesi\P300\FRSC\FRSCS002R03.dat'};%,'F:\Documentazione Tesi\P300\FRSC\FRSCS002R01.dat'};
% Mnt_File = 'E:\Documentazione Tesi\Montage\61ch.mmf';



Dat_Files={'C:\condiviso\P300\RIMA\RIMA000\RIMAS000R04.dat'};
Mnt_File = 'C:\Documents and Settings\pcabi\Desktop\e.fiorilla\TestData\Montage\61ch.mmf';




[BCI2k.Files, BCI2k.General] = BCI_LoadFiles(Dat_Files,Mnt_File);
clear Dat_Files;
clear Mnt_File;

BCI2k.Protocol = BCI_Protocol(BCI2k.Files.States, 'BCI_P300','[1 2]');

BCI2k.Conditioning = BCI_Conditioning('CarFilt',BCI2k.Files.Data,BCI2k.Files.Geom);

FExParams.EpochOverlap = 0;  %overlap automatico (Flash Flash ERP ERP...) 
FExParams.EpochLength = 0.6; %lunghezza del trial: 600 ms
FExParams.FSamp = str2double(BCI2k.Files.Parameters.SamplingRate.Value{1});

BCI2k.FeatExtraction = BCI_FeatExtractor('P300_FE','P300_Epocher',FExParams,BCI2k.Conditioning.FilteredSignal,BCI2k.Protocol.States);
clear FExParams;

StatResults = BCI_StatAnalysis('BCI_RSquare',BCI2k.FeatExtraction);
hfig = BCI_FeatVisualizer(StatResults);
% shading interp;


% FEXTR;

% [FeatMatrix(1),FeatMatrix(2)]=find(StatResults==-max(max(abs(StatResults))));
% FeatMatrix(3)=1;

FeatMatrix=[77,31,1
78,31,1
56,56,1
85,47,1
73,31,1
72,31,1]

PredWord =  BCI_P300_pred(BCI2k.FeatExtraction,BCI2k.Files.Parameters,FeatMatrix);

toc
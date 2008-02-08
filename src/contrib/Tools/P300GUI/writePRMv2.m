function writePRM(filename,dir1,MUD)

prmname=filename;
filename=[filename '.mud'];
numch=length(MUD.channels);

% generate prm fragment
MUD.MUD(:,4)=MUD.MUD(:,3);
MUD.MUD(:,3)=ones(size(MUD.MUD,1),1);
fid = fopen([dir1 prmname '.prm'], 'wt');

fprintf(fid,['Filtering:LinearClassifier matrix Classifier= ' num2str(size(MUD.MUD,1)) ' { input%%20channel input%%20element%%20(bin) output%%20channel 4 }']);
fprintf(fid,' %d %d %d %2.5f',reshape(MUD.MUD',1,size(MUD.MUD,1)*size(MUD.MUD,2)));
fprintf(fid,' // Linear classification matrix in sparse representation\n');
fprintf(fid,['Filtering:P3TemporalFilter int EpochLength= ' num2str(floor(MUD.windowlen(2)*1000/MUD.smprate)) 'ms 500ms 0 %% // Length of data epoch from stimulus onset\n']);
 fprintf(fid,['Source:gUSBampADC intlist SourceChDevices= 1 ' num2str(MUD.softwarech) ' ' num2str(MUD.softwarech) ' 1 ' num2str(MUD.softwarech) ' // number of digitized channels total\n']);
 
if MUD.SF==1
    CARmat=eye(MUD.softwarech)-ones(MUD.softwarech)/MUD.softwarech;
    CARmat=reshape(CARmat,1,MUD.softwarech^2);
    fprintf(fid,['Filtering:SpatialFilter matrix SpatialFilter= ' num2str(MUD.softwarech) ' ' num2str(MUD.softwarech)]);
    fprintf(fid,' %2.5f',CARmat);
else
    fprintf(fid,['Filtering:SpatialFilter matrix SpatialFilter= ' num2str(numch) ' ' num2str(numch)]);
    fprintf(fid,' %2.5f',reshape(eye(numch),1,numch^2));
end

fprintf(fid,' // columns represent input channels, rows represent output channels\n');
fprintf(fid,['Source:Online%%20Processing list TransmitChList= ' num2str(numch)]);
fprintf(fid,' %d',MUD.channels);
fprintf(fid,' 1 1 128 // list of transmitted channels\n');

fclose(fid);



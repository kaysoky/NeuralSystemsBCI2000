function [predicted,result,score]=P3Classify(Responses,Code,Type,MUD,NumberOfSequences,trialnr,NumMatrixRows,NumMatrixColumns,charvect,wind);
% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

windowlen=wind(2)-wind(1);
numresponse=size(Responses,1);
numchannels=size(Responses,3);

MUD(:,2)=MUD(:,2)-wind(1)+(MUD(:,1)-1)*windowlen;

nsamp=size(MUD(:,2),1);
response1=reshape(Responses,numresponse,numchannels*windowlen);
pscore=response1(:,MUD(:,2)+1)*MUD(:,3);

fprintf(1, 'Classifying Responses...\n\n');

% avg responses by row/col
choices=NumMatrixRows+NumMatrixColumns;
numletters=max(trialnr);

epoch=NumberOfSequences*choices;    
cflash=zeros(choices,NumberOfSequences);
score=zeros(numletters,choices);

cc=1;
for bb=1:numletters
    range=find(trialnr==bb);
    if ~isempty(range)
        range=range(1:epoch);
        codes=unique(Code(range).*Type(range));

        if length(codes)>=2
            codecol(cc)=codes(2);
            if length(codes)>2
                coderow(cc)=codes(3);
            end

            for aa=1:choices
                cflash(aa,:)=pscore(range(find(Code(range)==aa)));
                cflash(aa,:)=cumsum(cflash(aa,:));
            end

            score(cc,:)=cflash(:,NumberOfSequences);

            [valc,maxcol]=max(cflash(1:NumMatrixColumns,:));
            [valr,maxrow]=max(cflash(NumMatrixColumns+1:choices,:));

            if NumMatrixRows==0
                predictedcol(cc,:)=maxcol;
                predictedrow=[];
            else
                predictedcol(cc,:)=maxcol;
                predictedrow(cc,:)=maxrow+NumMatrixColumns;
            end
            cc=cc+1;
        end
    end
end

% if size(predictedcol,1)~=size(targetletter',1)|size(predicted,2)~=size(targetletter',2)
%     error('Number of predicted charcters does not match text to spell.');
% end

if NumMatrixRows==0
    tar=charvect(codecol,1);
    codecol=repmat(codecol',1,NumberOfSequences);
    numcorrect=[predictedcol==codecol];
    predicted=reshape(charvect(predictedcol,1),cc-1,NumberOfSequences);      
else
    tar=charvect((NumMatrixColumns*(coderow-1-NumMatrixColumns))+codecol);
    coderow=repmat(coderow',1,NumberOfSequences);
    codecol=repmat(codecol',1,NumberOfSequences);
    numcorrect=[predictedrow==coderow & predictedcol==codecol];
    predicted=charvect((NumMatrixColumns*(predictedrow-1-NumMatrixColumns))+predictedcol);
end

if size(numcorrect,1)>1
    numcorrect=sum(numcorrect);
end

result=numcorrect/double(cc-1)*100;

fprintf(1, '                Target: %s \n',tar);
fprintf(1, '\n  Flashes | %% Correct | Predicted Symbols' );
fprintf(1, '\n  --------|-----------|------------------\n' );
for kk=1:length(result)
    fprintf(1, '    %02d    |    %03d%%   | %s \n',kk,round(result(kk)),predicted(:,kk)');
end
fprintf(1, '\n\n')

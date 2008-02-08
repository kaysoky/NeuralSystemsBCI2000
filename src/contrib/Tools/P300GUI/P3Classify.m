function [predicted,result,score,resultthresh]=P3Classify(Responses,Code,Type,MUD,NumberOfSequences,trialnr,NumMatrixRows,NumMatrixColumns,charvect,wind);

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
lrv=zeros(1,choices);

cc=1;
for bb=1:numletters
    cflash=zeros(choices,NumberOfSequences);
    range=find(trialnr==bb);
    if ~isempty(range)
%         range=range(1:epoch);
        codes=unique(Code(range).*Type(range));

        if length(codes)>=2
            codecol(cc)=codes(2);
            if length(codes)>2
                coderow(cc)=codes(3);
            end

            for aa=1:choices
                rv=pscore(range(find(Code(range)==aa)));
                lrv(aa)=length(rv);
                cflash(aa,1:lrv(aa))=rv;
                cflash(aa,:)=cumsum(cflash(aa,:));                
            end
            
            numseq(bb)=min(lrv);
            
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
                        %-------------------------------------------
            % Dynamical Thresholding codes
            %-------------------------------------------
            
                       threshvec=[0.5:0.5:10];
            for iterthresh=1:length(threshvec)
                Thresh=threshvec(iterthresh);
                for ii=1:NumberOfSequences
                    auxcol=cflash(1:NumMatrixColumns,ii);
                    sorted=sort(auxcol);
                    differencecol=diff(sorted);


                    auxlig=cflash(NumMatrixColumns+1:choices,ii);
                    sorted=sort(auxlig);
                    differencelig=diff(sorted);
                    % here is the main heuritics

                    MeanDiffcol=median(differencecol(1:NumMatrixColumns-2));
                    MeanDifflig=median(differencelig(1:NumMatrixRows-2));
                    if differencecol(NumMatrixColumns-1)>Thresh*MeanDiffcol & differencelig(NumMatrixRows-1)>Thresh*MeanDifflig
                        optimalnbshot(cc,iterthresh)=ii;
                        break
                    else
                        optimalnbshot(cc,iterthresh)=NumberOfSequences;
                    end;
                end;

            end;
            %-------------------------------------------------------
            
            
            cc=cc+1;
        end
    end
end

% if size(predictedcol,1)~=size(targetletter',1)|size(predicted,2)~=size(targetletter',2)
%     error('Number of predicted charcters does not match text to spell.');
% end

if NumMatrixRows==0
    tar=charvect(codecol,:);
    codecol=repmat(codecol',1,NumberOfSequences);
    numcorrect=[predictedcol==codecol];
    predicted=charvect(predictedcol,:)';
else
    tar=charvect((NumMatrixColumns*(coderow-1-NumMatrixColumns))+codecol);
    coderow=repmat(coderow',1,NumberOfSequences);
    codecol=repmat(codecol',1,NumberOfSequences);
    numcorrect=[predictedrow==coderow & predictedcol==codecol];
    predicted=charvect((NumMatrixColumns*(predictedrow-1-NumMatrixColumns))+predictedcol);
end

%-----------------------
% Alain's modification
%------------------------
numcorrectmat=numcorrect;
%-------------------------
if size(numcorrect,1)>1
    numcorrect=sum(numcorrect);
end

result=numcorrect/double(cc-1)*100;


fprintf(1, '                Target: %s \n',tar);
fprintf(1, '\n  Flashes | %% Correct | Predicted Symbols' );
fprintf(1, '\n  --------|-----------|------------------\n' );
for kk=1:length(result)
    pred=predicted(:,kk)';
    rind=find(numseq<kk);
    if ~isempty(rind)
       for uu=1:length(rind)
           pred(rind(uu))=' ';
       end
    end
    fprintf(1, '    %02d    |    %03d%%   | %s \n',kk,round(result(kk)),pred);
end
fprintf(1, '\n\n')

%-----------------------------------
%       Alain's modification
%-----------------------------------
fprintf('-------------------------------------------------\n');
fprintf('               Dynamical Sequences               \n');
fprintf('%s \t\t %s \t\t %s\n','Thresh', 'Aver. Seq', 'Perf' );
fprintf('-------------------------------------------------\n');

for iterthresh=1:length(threshvec)
    PC=0;
    for ii=1:size(optimalnbshot,1)
        PC=PC+numcorrectmat(ii,optimalnbshot(ii,iterthresh))/double(size(optimalnbshot,1));
    end;
    fprintf('%2.2f        %2.2f        %2.2f \n', threshvec(iterthresh), mean(optimalnbshot(:,iterthresh)),PC*100);
    resultthresh(iterthresh,:)=[threshvec(iterthresh)  mean(optimalnbshot(:,iterthresh)) PC*100];
end;
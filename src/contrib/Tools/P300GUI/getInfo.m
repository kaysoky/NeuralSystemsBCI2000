function [signal,state,parms]=getInfo(traindatfiles,traindatdir,channels)

fprintf(1,'Loading files...\n')

statestr={'Flashing' 'PhaseInSequence' 'StimulusType' 'StimulusCode' 'SelectedRow' 'SelectedColumn' 'SelectedTarget' 'StimulusBegin'};
numstat=length(statestr);
state=struct;
for dd=1:numstat
    state.(char(statestr(dd)))= [];
end


if isempty(traindatfiles)
    [traindatfiles,traindatdir]=uigetfile('*.dat','Select the P300 ASCII(.dat) training data file(s)','multiselect','on');
end

if iscell(traindatfiles)==0
    traindatfiles={traindatfiles};
end

numstatp3av=numstat;
numtrain=length(traindatfiles);
signal=[];
state.trialnr=[];
for kk=1:numtrain
    fprintf(1,'   run %d...\n',kk)
    [sig,sts,prm] = load_bcidat([traindatdir char(traindatfiles(kk))]);   
    if ~strcmp(class(sts.StimulusType),class(sts.StimulusCode))
        sts.StimulusType=cast(sts.StimulusType,class(sts.StimulusCode));
    end
    sig=single(sig);
    NSamples=length(sig); 
    Gains   = repmat((prm.SourceChGain.NumericValue), [1 NSamples])';
    Offsets = repmat((prm.SourceChOffset.NumericValue), [1 NSamples])';
    Gains=single(Gains);
    Offsets=single(Offsets);
    sig = Gains .* (sig-Offsets);
    signal=cat(1,signal,sig);    
    parms.SoftwareCh(kk)=size(signal,2);
    parms.SamplingRate(kk)=(prm.SamplingRate.NumericValue);
    
    if isfield(prm,'OffTime')
        OffTime=prm.OffTime.NumericValue;
        OnTime=prm.OnTime.NumericValue;
        PreSetInterval=prm.PreSetInterval.NumericValue;
        PostSetInterval=prm.PostSetInterval.NumericValue;
        if isfield(prm,'TargetDefinitionMatrix')
            TargetDefinitionMatrix=prm.TargetDefinitionMatrix.Value;
        end
        v2=0;
    else
        OffTime=prm.ISIMinDuration.NumericValue;
        OnTime=prm.StimulusDuration.NumericValue;
        PreSetInterval=prm.PreSequenceDuration.NumericValue;
        PostSetInterval=prm.PostSequenceDuration.NumericValue;
        TargetDefinitionMatrix=prm.TargetDefinitions.Value;
        v2=1;
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
    parms.ISI=(prm.SampleBlockSize.NumericValue)*(OffTime+OnTime)*1000/parms.SamplingRate(kk);
    [x,ok]=str2num(char(PostSetInterval));
    
    if ok==1
        parms.PostSetInterval=(prm.SampleBlockSize.NumericValue)*PostSetInterval*1000/parms.SamplingRate(kk);
        parms.PreSetInterval=(prm.SampleBlockSize.NumericValue)*PreSetInterval*1000/parms.SamplingRate(kk);
    else
%         PostSetInterval=char(PostSetInterval);
%         PreSetInterval=char(PreSetInterval);
%         parms.PostSetInterval=str2num((PostSetInterval(1:length(PostSetInterval)-1)));
%         parms.PreSetInterval=str2num((PreSetInterval(1:length(PreSetInterval)-1)));
         parms.PostSetInterval=PostSetInterval;
         parms.PreSetInterval=PreSetInterval;
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   if isfield(prm, 'InterpretMode') | isfield(prm, 'TextToSpell')
%     if isfield(prm, 'InterpretMode')       % P3AV
%         av=1;
%         numstat=numstatp3av-3;
% 
%         if length(unique((prm.Sequence.NumericValue)))~=1
%             fprintf(1,'Warning: Classification will not work with P300 oddball data.\n')
%         end
% 
%         parms.NumMatrixRows(kk)=0;
%         parms.NumMatrixColumns(kk)=length((prm.Sequence.NumericValue));
% 
%         tval=char(prm.Matrix(1,:)');
% 
%         TargetDef(kk)={tval};
%         parms.NumberOfSequences(kk)=(prm.NumberOfSeq.NumericValue);
% 
%         indx=(prm.ToBeCopied.NumericValue);
%         parms.target(kk)={tval(indx,1)};
% 
%         samps=size(sig,1);
%         trialnr=kk*ones(samps,1);
%         state.trialnr=cat(1,state.trialnr,trialnr);

%     elseif  isfield(prm, 'TextToSpell') % P3 Speller

        av=0;
        if isfield(prm, 'OnlineMode')
            if char(prm.OnlineMode.NumericValue)=='0'
                %%%%space
                cc=[];
                spchar={'%20' '%f6' '%d6' '%e4' '%c4' '%fc' '%dc' '%eb' '%ec'};
                repchar={' ' '�' '�' '�' '�' '�' '�' '�' '�'};
                TextToSpell=char(prm.TextToSpell.Value);
                for qq=1:size(spchar,2)
                    spce=strfind(TextToSpell,char(spchar(qq)));
                    if ~isempty(spce)
                        cc=1;
                        plchld=[];
                        for yy=1:length(spce)
                            newtar=[TextToSpell(cc:spce(yy)-1) char(repchar(qq))];
                            target=strcat(plchld,newtar);
                            plchld=target;
                            cc=spce(yy)+3;
                        end
                        newtar=TextToSpell(cc:length(TextToSpell));
                        parms.target(kk)={strcat(plchld,newtar)};
                    end
                end

                if isempty(cc)
                    parms.target(kk)={TextToSpell};
                end

            end
        end
        parms.NumberOfSequences(kk)=(prm.NumberOfSequences.NumericValue);
        if isfield(prm, 'NumMatrixRows')
            parms.NumMatrixRows(kk)=(prm.NumMatrixRows.NumericValue);
            parms.NumMatrixColumns(kk)=(prm.NumMatrixColumns.NumericValue);

            spchar={'%20' '%f6' '%d6' '%e4' '%c4' '%fc' '%dc' '%eb' '%ec'};
            repchar={' ' '�' '�' '�' '�' '�' '�' '�' '�'};
            TDef=char(TargetDefinitionMatrix(:,2));
            %             TDef=TDef(:,1);

            for qq=1:size(spchar,2)
                fspce=strmatch(char(spchar(qq)),char(TargetDefinitionMatrix(:,2)),'exact');
                if ~isempty(fspce)
                    TDef(fspce)=char(repchar(qq));
                end
            end
            TargetDef(kk)={TDef};
        else
            parms.NumMatrixRows(kk)=6;
            parms.NumMatrixColumns(kk)=6;
            TargetDef(kk)={char('A','B','C','D','E','F',...
                'G','H','I','J','K','L',...
                'M','N','O','P','Q','R',...
                'S','T','U','V','W','X',...
                'Y','Z','1','2','3','4',...
                '5','6','7','8','9','_')};
        end
    else
        error('Data file must be valid P3Speller or P3AV format')
    end
   
    for zz=1:numstat
        if isfield(sts,char(statestr(zz)))
        idx1= strmatch(char(statestr(zz)),sts,'exact');
        state.(char(statestr(zz)))=cat(1,state.(char(statestr(zz))),sts.(char(statestr(zz))));
        end
    end   
end

clear sig sts

if av==0;
    samps=size(signal,1);
    indx=find(state.PhaseInSequence(1:samps-1)==1 & state.PhaseInSequence(2:samps)==2)+1;
    state = rmfield(state,'PhaseInSequence');
    state.trialnr=zeros(samps,1);
    state.trialnr(indx)=ones(1,length(indx));
    state.trialnr=cumsum(state.trialnr);
    state.trialnr=int16(state.trialnr);
end


parms.TargetDef=TargetDef{1};
parms.NumberOfSequences=min(parms.NumberOfSequences);
parms.SoftwareCh=unique(parms.SoftwareCh);
parms.NumMatrixRows=unique(parms.NumMatrixRows);
parms.NumMatrixColumns=unique(parms.NumMatrixColumns);
parms.SamplingRate=unique(parms.SamplingRate);
parms.trainfiles=traindatfiles;



if isfield(prm, 'TextToSpell') & isfield(prm, 'OnlineMode') & char(prm.OnlineMode.NumericValue)=='1'
    fprintf(1,'\n*********************************************************')
    fprintf(1,'\nWarning: Online Mode - Results are based on the assumtion')
    fprintf(1,'\n                       that the selected cell is correct.')
    fprintf(1,'\n**********************************************************\n\n')

    nrow=parms.NumMatrixRows;
    ncol=parms.NumMatrixColumns;

    [scolmat,srowmat]=ind2sub([ncol,nrow],reshape(1:nrow*ncol,ncol,nrow));
    numletters=max(state.trialnr);
    pause=zeros(1,numletters);
    sleep=zeros(1,numletters);
    for bb=1:numletters-1
        range=find(state.trialnr==bb);

        tvalue=unique(state.SelectedTarget(max(range)+1));

        if tvalue(1)~=0
            scol=scolmat(tvalue(1));
            srow=srowmat(tvalue(1))+ncol;           
            %         scol=unique(state.SelectedColumn(max(range)+1));
            %         srow=unique(state.SelectedRow(max(range)+1))+ncol;

            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %        Pause for SAM
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%             if scol==7 & srow==14
%                 pause(bb+1)=1;
%                 if pause(bb)==1
%                     sleep(bb)=0;
%                 else
%                     sleep(bb+1)=1;
%                 end
%             else
%                 sleep(bb+1)=sleep(bb);
%             end
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %        Pause for SAM
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            if sleep(bb)==0
                indc=find(state.StimulusCode(range)==scol(1));
                indr=find(state.StimulusCode(range)==srow(1));
                state.StimulusType(range(indr))=int16(ones(1,length(indr)));
                state.StimulusType(range(indc))=int16(ones(1,length(indc)));
            else
                state.Flashing(range)=zeros(1,length(range));
            end

        else
            state.Flashing(range)=zeros(1,length(range));
        end

    end
    range=find(state.trialnr==numletters);
    state.Flashing(range)=zeros(1,length(range));    
end

if isempty(state.Flashing)
    state.Flashing=state.StimulusBegin;
    indx1=find(state.StimulusCode<=parms.NumMatrixRows & state.StimulusCode~=0);
    indx2=find(state.StimulusCode>parms.NumMatrixRows & state.StimulusCode~=0);
    state.StimulusCode(indx1)=state.StimulusCode(indx1)+parms.NumMatrixColumns;
    state.StimulusCode(indx2)=state.StimulusCode(indx2)-parms.NumMatrixRows;
end
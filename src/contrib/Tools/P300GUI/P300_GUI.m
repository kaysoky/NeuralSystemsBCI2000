function varargout = P300_GUI(varargin)
% P300_GUI M-file for P300_GUI.fig
%      P300_GUI, by itself, creates a new P300_GUI or raises the existing
%      singleton*.
%
%      H = P300_GUI returns the handle to a new P300_GUI or the handle to
%      the existing singleton*.
%
%      P300_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in P300_GUI.M with the given input arguments.
%
%      P300_GUI('Property','Value',...) creates a new P300_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before P300_GUI_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to P300_GUI_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES
% Copyright 2002-2003 The MathWorks, Inc.
% Edit the above text to modify the response to help P300_GUI
% Last Modified by GUIDE v2.5 15-Jan-2008 10:46:48

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @P300_GUI_OpeningFcn, ...
    'gui_OutputFcn',  @P300_GUI_OutputFcn, ...
    'gui_LayoutFcn',  [] , ...
    'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before P300_GUI is made visible.
function P300_GUI_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% Choose default command line output for P300_GUI
handles.output = hObject;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% start default parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
handles.windlen=[0 800];
handles.DFfreq=[20];
handles.maxiter=60;
handles.Ch1=[1:8];
handles.Ch2=[1:8];
% handles.Ch1=[34 11 51 56 62 60 49 53];
% handles.Ch2=[56 62 60];
% handles.Ch1=[13:16];
% handles.Ch2=[3:8 11:16];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% handles.Ch1=[34 11 51 62 56 60];
% handles.Ch2=[34 4 11 18 51 9 13 47 49 53 55 56:63];
% handles.Ch1=[34 11 51 62 9 13 49 53 56 60];
% handles.Ch2=[34 11 51 62 9 13 49 53 56 60 3 5 17 19 32 36];
handles.penter=.1;
handles.premove=.15;
handles.enableCh2=0;
handles.xls=0;
handles.stde=0;
handles.SF=2;
handles.MUDprefix='MUD';
handles.testindx=0;
handles.MUDindx=0;
handles.rndsmp=100;
handles.method=1;

set(handles.windowedit, 'String',num2str(handles.windlen));
set(handles.DFedit, 'String',num2str(handles.DFfreq));
set(handles.Ch1edit, 'String',num2str(handles.Ch1));
set(handles.Ch2edit, 'String',num2str(handles.Ch2));
set(handles.swldaedit, 'String',num2str(handles.maxiter));
set(handles.rndedit, 'String',num2str(handles.rndsmp));
set(handles.status, 'String', 'Ready');

handles.plots.clr=['b' 'r' 'g' 'c' 'm' 'y' 'k'];
handles.plots.mark=['.' 'o' 'x' '+' '*' 's' 'd' 'v' '^' '>' '<' 'p' 'h'];
handles.plots.dsh=['-' ':' '-.' '--'];
handles.plots.wth=[1 2 3];

% Update handles structure
guidata(hObject, handles);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% end default parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% UIWAIT makes P300_GUI wait for user response (see UIRESUME)
% uiwait(handles.figure1);
% --- Outputs from this function are returned to the command line.
function varargout = P300_GUI_OutputFcn(hObject, eventdata, handles)
varargout{1} = handles.output;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% start edit windows
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function Ch2edit_Callback(hObject, eventdata, handles)
handles.Ch2=str2num(get(hObject,'String'));
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function Ch2edit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function windowedit_Callback(hObject, eventdata, handles)
handles.windlen=str2num(get(hObject,'String'));

if length(handles.windlen)==1
    handles.windlen=[0 handles.windlen];
end
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function windowedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


function swldaedit_Callback(hObject, eventdata, handles)
handles.maxiter=str2num(get(hObject,'String'));
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function swldaedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function Ch1edit_Callback(hObject, eventdata, handles)
handles.Ch1=str2num(get(hObject,'String'));
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function Ch1edit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function DFedit_Callback(hObject, eventdata, handles)
handles.DFfreq=str2num(get(hObject,'String'));
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function DFedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function prefixedit_Callback(hObject, eventdata, handles)
handles.MUDprefix=get(hObject,'String');
guidata(hObject,handles)
function prefixedit_CreateFcn(hObject, eventdata, handles)
% --- Executes during object creation, after setting all properties.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function mudnum_Callback(hObject, eventdata, handles)
handles.nummud=str2num(get(hObject,'String'));
set(handles.saveMUDbutton, 'Enable','on');
set(handles.savePRMbutton, 'Enable','on');
set(handles.prmv2, 'Enable','on');
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function mudnum_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function topotimeedit_Callback(hObject, eventdata, handles)
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function topotimeedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function respchedit_Callback(hObject, eventdata, handles)
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function respchedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function topoaxisedit_Callback(hObject, eventdata, handles)
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function topoaxisedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function respaxisedit_Callback(hObject, eventdata, handles)
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function respaxisedit_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


function rndedit_Callback(hObject, eventdata, handles)
handles.rndsmp=str2num(get(hObject,'String'));
guidata(hObject,handles)
function rndedit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rndedit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% end edit windows
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% start buttons
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% --- Executes on button press in gettrain.
function gettrain_Callback(hObject, eventdata, handles)
[handles.traindatfiles,handles.traindatdir]=uigetfile('*.dat','Select the BCI2000 P300 (.dat) training data file(s)','multiselect','on');

if iscell(handles.traindatfiles),
    handles.traindatfiles = sort(handles.traindatfiles);
end

if iscell(handles.traindatfiles)
    tname=char(handles.traindatfiles(1));
    handles.trainfile=tname(1:length(tname)-7);
    numtrain=length(handles.traindatfiles);
else
    handles.trainfile=handles.traindatfiles(1:length(handles.traindatfiles)-7);
    numtrain=1;
end
set(handles.runbutton, 'Enable','on');
set(handles.chvstimebutton, 'Enable','on');
set(handles.trainfiletxt, 'String',['Training set: ' handles.trainfile ' (' num2str(numtrain) ' runs)']);

guidata(hObject,handles)

% --- Executes on button press in gettest.
function gettest_Callback(hObject, eventdata, handles)
[testfiles,testdir]=uigetfile('*.dat','Select the BCI2000 P300 (.dat) test data file(s)','MultiSelect', 'on');

if testdir~=0
    handles.testindx=handles.testindx+1;
    handles.testfilesall(handles.testindx)={testfiles};
    handles.testdirall(handles.testindx)={testdir};
    if iscell(testfiles)
        tname=char(testfiles(1));
        handles.testfile(handles.testindx)={tname(1:length(tname)-7)};
        numtest(handles.testindx)=length(testfiles);
    else
        handles.testfile(handles.testindx)={testfiles(1:length(testfiles)-7)};
        numtest=1;
    end

    set(handles.testfiletxt, 'String',handles.testfile);

    if handles.MUDindx>0 & handles.testindx>0
        set(handles.applybutton, 'Enable','on');
    end
    set(handles.cleartestbutton, 'Enable','on');

end
guidata(hObject,handles)

% --- Executes on button press in ldmudbutton.
function ldmudbutton_Callback(hObject, eventdata, handles)
[MUDnames,MUDdir]=uigetfile('*.mud','Select MUD file(s)','MultiSelect', 'on');

if MUDdir~=0
    if ~iscell(MUDnames)
        MUDnames={MUDnames};
    end
    nnewMUD=size(MUDnames,2);
    for qq=1:nnewMUD
        nnme=char(MUDnames(qq));
        nnme=nnme(1:length(nnme)-4);
        [tMUD]=loadMUD(MUDdir,char(MUDnames(qq)));
        handles.tlabel(handles.MUDindx+qq)={[num2str(handles.MUDindx+qq) ': ' nnme]};
        handles.tlabel2(handles.MUDindx+qq)={nnme};
        handles.MUD(handles.MUDindx+qq)=tMUD;
    end

    set(handles.weightlist, 'String', handles.tlabel)
    handles.MUDindx=handles.MUDindx+nnewMUD;

    if handles.MUDindx>0 & handles.testindx>0
        set(handles.applybutton, 'Enable','on');
    end
    if handles.MUDindx>0
        set(handles.clearmudbutton, 'Enable','on');
    end
end

guidata(hObject,handles)

% --- Executes on button press in runbutton.
function runbutton_Callback(hObject, eventdata, handles)



[signal,state,parms]=getInfo(handles.traindatfiles,handles.traindatdir,[]);
handles.windowlen=round(handles.windlen*parms.SamplingRate/1000);
set(handles.sampratetxt, 'String', ['Sampling Rate: ' num2str(parms.SamplingRate) ' Hz']);
set(handles.numflashtxt, 'String', ['# Sequences: ' num2str(parms.NumberOfSequences)]);
set(handles.numchtxt, 'String', ['# Channels: ' num2str(parms.SoftwareCh)]);


handles.DF=ceil(parms.SamplingRate./(handles.DFfreq+.000001));
handles.MA=handles.DF;


if parms.SoftwareCh<max(handles.Ch1)
    error('*** Training data has less channels than Channel Set 1 ***')
end

if handles.enableCh2==1
    if parms.SoftwareCh<max(handles.Ch2)
        error('*** Training data has less channels than Channel Set 2 ***')
    end
    Ch2=handles.Ch2;
else
    Ch2=[];
end

chset=unique([handles.Ch1 Ch2]);
[a,ind1]=ismember(handles.Ch1,chset);
[a,ind2]=ismember(handles.Ch2,chset);

chSW={handles.Ch1 Ch2};
chidx={ind1 ind2};

switch handles.SF
    case 1
        opt=1:2;
    case 2
        opt=1;
    case 3
        opt=2;
end

switch handles.method
    case 1
        meth='SW';
    case 2
        meth='LS';
    case 3
        meth='LG';
end

%  figure
pind=handles.MUDindx;
zz=1;
for xsf=opt
    if xsf==2
        signal=CARfilter(signal);
    end
    [Responses]=GetP3Responses(signal(:,chset),state.trialnr,handles.windowlen,state.StimulusCode,state.StimulusType,state.Flashing,chset,xsf-1,handles.rndsmp);
    for xch=1:handles.enableCh2+1
        for xma=1:length(handles.MA)
            fprintf(1, '\n***************************************************************\n' );
            fprintf(1, [handles.trainfile ' to ' handles.trainfile] );
            fprintf(1, '\n***************************************************************\n\n' );
            fprintf(1,['Ch Set: ' num2str(cell2mat(chSW(xch))) '\n']);
            fprintf(1,['Method: ' meth ' CAR: ' num2str(xsf-1) ' DF: ' num2str(handles.DFfreq(xma)) ' RS: ' num2str(handles.rndsmp) '\n\n']);
            [tMUD]=SWLDA(Responses.Responses(:,:,cell2mat(chidx(xch))),Responses.Type,handles.MA(xma),handles.DF(xma),handles.windowlen,cell2mat(chSW(xch)),xsf-1,handles.rndsmp,handles.trainfile,parms.SamplingRate,handles.penter,handles.premove,handles.maxiter,parms.SoftwareCh,handles.method);
            if ~isempty(tMUD.MUD)
                handles.MUDindx=handles.MUDindx+1;
                [a,indm]=ismember(tMUD.channels,chset);
                handles.MUD(handles.MUDindx)=tMUD;
                handles.tlabel(handles.MUDindx)={[num2str(handles.MUDindx) ': ChS' num2str(xch) 'CAR' num2str(xsf-1) 'DF' num2str(handles.DFfreq(xma)) 'RS' num2str(handles.rndsmp) meth]};
                handles.tlabel2(handles.MUDindx)={['ChS' num2str(xch) 'CAR' num2str(xsf-1) 'DF' num2str(handles.DFfreq(xma)) 'RS' num2str(handles.rndsmp) meth]};
                set(handles.mudnum, 'Enable','on');

                if handles.testindx~=0
                    set(handles.applybutton, 'Enable','on');
                end

                if handles.MUDindx>0
                    set(handles.weightlist, 'String', handles.tlabel)
                    set(handles.clearmudbutton, 'Enable','on');
                end
                guidata(hObject,handles)
                if handles.rndsmp==100
                    [predicted,result(:,zz),score,resultthresh]=P3Classify(Responses.Responses(:,:,indm),Responses.Code,Responses.Type,tMUD.MUD,parms.NumberOfSequences,Responses.trial,parms.NumMatrixRows,parms.NumMatrixColumns,parms.TargetDef,tMUD.windowlen);
                end
                zz=zz+1;
            end
        end
    end
end

clear signal
set(handles.status, 'String', 'Finished generating weights');

if handles.MUDindx>pind & handles.rndsmp==100

    figure
    set(gcf,'Name',[handles.trainfile ' to ' handles.trainfile])
    subplot(2,1,1)
    plot(result,'linewidth',2)
    set(gca,'YAxisLocation','right')
    axis([1 parms.NumberOfSequences 0 100])
    title([handles.trainfile ' to ' handles.trainfile])
    xlabel('# Sequences')
    ylabel('Percent Correct')
    legend(handles.tlabel(pind+1:handles.MUDindx),'Location','BestOutside')
    %AR%
    hold on;
    plot(resultthresh(:,2),resultthresh(:,3),'r','Linewidth',2);


    %     subplot(2,1,2)
    %     ribbon(result)
    %     axis([.5  handles.MUDindx-pind+.5 1 parms.NumberOfSequences 0 100])
    %     view(37.5,30)
    %     set(gca,'YAxisLocation','right')
    %     ylabel('# Sequences')
    %     xlabel('MUD')
    %     zlabel('Percent Correct')
    %     legend(handles.tlabel(pind+1:handles.MUDindx),'Location','BestOutside')

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   
    p=result/100-.0001;
    n=parms.NumMatrixRows*parms.NumMatrixColumns;
    br=log2(n)+p.*log2(p)+(1-p).*log2((1-p)/(n-1));
    bx=br/log2(n); % symbols per trial
    exval=2*bx-1;
  
        
    time=[1:parms.NumberOfSequences]'*parms.ISI*(parms.NumMatrixRows+parms.NumMatrixColumns)/60000;
    time=time+(parms.PostSetInterval+parms.PreSetInterval)/60000;
    indx=find(exval<=0);
    exval(indx)=zeros(1,length(indx));

    for nn=1:size(result,2)
        exval(:,nn)=exval(:,nn)./time;
    end

    subplot(2,1,2)
    plot(exval,'linewidth',2)
    set(gca,'YAxisLocation','right')
    maxi=max(max(exval));
    if maxi==0 
        maxi=1;
    end;
    axis([1 parms.NumberOfSequences 0 maxi])
    xlabel('# Sequences')
    ylabel('Symbols/Minute')
    legend(handles.tlabel(pind+1:handles.MUDindx),'Location','BestOutside')
    
    %-------------------------------------------
    % AR
    %------------------------------------------
    p=resultthresh(:,3)/100-.0001;
    n=parms.NumMatrixRows*parms.NumMatrixColumns;
    br=log2(n)+p.*log2(p)+(1-p).*log2((1-p)/(n-1));
    bx=br/log2(n); % symbols per trial
    exval=2*bx-1;
    
    time=[resultthresh(:,2)]*parms.ISI*(parms.NumMatrixRows+parms.NumMatrixColumns)/60000;
    time=time+(parms.PostSetInterval+parms.PreSetInterval)/60000;
    indx=find(exval<=0);
    exval(indx)=zeros(1,length(indx));
    exval=exval./time;
    subplot(2,1,2); hold on
    plot(resultthresh(:,2),exval,'r','linewidth',2)
    [maxi2,ind]=max(exval);
    if maxi2==0 
        maxi2=1;
    end;
    axis([1 parms.NumberOfSequences 0 max([maxi maxi2])])    
    fprintf('-----------------------------------------\n');
    fprintf('Optimal value of T = %2.2f\n',resultthresh(ind,1));
    fprintf('-----------------------------------------\n');
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    if handles.xls==1
        train=[handles.trainfile 'train'];
        writeXLS(result,handles.trainfile,train,handles.traindatdir,handles.tlabel(pind+1:handles.MUDindx),zz-1);
    end

    plotresults(result,handles.MUD(pind+1:handles.MUDindx),handles.trainfile,handles.trainfile)

end


guidata(hObject,handles)

% --- Executes on button press in applybutton.
function applybutton_Callback(hObject, eventdata, handles)

if handles.testindx~=0

    if ~isfield(handles,'trainfile')
        handles.trainfile=handles.testfile{1};
        handles.traindatdir=handles.testdirall{1};
    end

    gMUD=struct2cell(handles.MUD);
    MA=cell2mat(gMUD(4,:));
    DF=cell2mat(gMUD(5,:));
    xm=unique(MA);
    xd=unique(DF);
    minseq=100000;
    grandavg=[];
    for rr=1:handles.testindx % each test file
        % figure
        [signal,state,parms]=getInfo(handles.testfilesall{rr},handles.testdirall{rr},[]);
        result=[];
        if parms.NumberOfSequences<minseq
            minseq=parms.NumberOfSequences;
        end
        for bb=1:handles.MUDindx % each mud file
            tMUD=handles.MUD(bb);

            if parms.SoftwareCh<max(tMUD.channels)
                error('*** Test data has less channels specified in the MUD ***')
            end

            if tMUD.SF==1
                signal1=CARfilter(signal);
                signal1=signal1(:,tMUD.channels);
            else
                signal1=signal(:,tMUD.channels);
            end

            fprintf(1, '\n***************************************************************\n' );
            fprintf(1, [handles.tlabel{bb} ' to ' handles.testfile{rr}]);
            fprintf(1, '\n***************************************************************\n\n' );
            fprintf(1,['Ch Set: ' num2str(tMUD.channels) '\n']);
            fprintf(1,['CAR: ' num2str(tMUD.SF) ' DF: ' num2str(round(tMUD.smprate/tMUD.DF)) ' RS: ' num2str(tMUD.RS) '\n\n']);

            [Responses]=GetP3Responses(signal1,state.trialnr,tMUD.windowlen,state.StimulusCode,state.StimulusType,state.Flashing,tMUD.channels,tMUD.SF,100);
            [predicted,result(:,bb),score,resultthresh] =P3Classify(Responses.Responses,Responses.Code,Responses.Type,tMUD.MUD,parms.NumberOfSequences,Responses.trial,parms.NumMatrixRows,parms.NumMatrixColumns,parms.TargetDef,tMUD.windowlen);

            if parms.SamplingRate~=tMUD.smprate
                fprintf(1, '***********************************************************\n' );
                fprintf(1,   '***  Warning: Training and test sample rates differ!!!  ***\n');
                fprintf(1,   '***           Classification may be incorrect           ***');
                fprintf(1, '\n***********************************************************\n' );
            end

            [a,indh1]=find(xm==tMUD.MA);
            [a,indh2]=find(xd==tMUD.DF);

        end % each mud file

        if handles.xls==1;
            test=[handles.testfile{rr} 'test'];
            writeXLS(result,handles.trainfile,test,handles.traindatdir,handles.tlabel,handles.MUDindx);
        end

        figure
        set(gcf,'Name',['All MUDs to ' handles.testfile{rr}])
        subplot(2,1,1)
        plot(result,'linewidth',2)
        set(gca,'YAxisLocation','right')
        axis([1 parms.NumberOfSequences 0 100])
        title(['All MUDs to ' handles.testfile{rr}])
        xlabel('# Sequences')
        ylabel('Percent Correct')
        legend(handles.tlabel,'Location','BestOutside')
        %AR%
        hold on;
        plot(resultthresh(:,2),resultthresh(:,3),'r','Linewidth',2);

        %         subplot(2,1,2)
        %         ribbon(result)
        %         axis([.5 handles.MUDindx+.5 1 parms.NumberOfSequences 0 100])
        %         view(37.5,30)
        %         set(gca,'YAxisLocation','right')
        %         ylabel('# Sequences')
        %         xlabel('MUD')
        %         zlabel('Percent Correct')
        %         legend(handles.tlabel,'Location','BestOutside')
        %

        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        p=result/100-.0001;
        n=parms.NumMatrixRows*parms.NumMatrixColumns;
        br=log2(n)+p.*log2(p)+(1-p).*log2((1-p)/(n-1));
        bx=br/log2(n); % symbols per trial
        exval=2*bx-1;
        time=[1:parms.NumberOfSequences]'*parms.ISI*(parms.NumMatrixRows+parms.NumMatrixColumns)/60000;
        time=time+(parms.PostSetInterval+parms.PreSetInterval)/60000;
        indx=find(exval<=0);
        exval(indx)=zeros(1,length(indx));
        for nn=1:size(result,2)
            exval(:,nn)=exval(:,nn)./time;
        end

        subplot(2,1,2)
        plot(exval,'linewidth',2)
        set(gca,'YAxisLocation','right')
        maxi=max(max(exval));
        if maxi==0 
            maxi=1;
        end;
        axis([1 parms.NumberOfSequences 0 maxi])
        xlabel('# Sequences')
        ylabel('Symbols/Minute')
        legend(handles.tlabel,'Location','BestOutside')
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %-------------------------------------------
        % AR
        %------------------------------------------
        p=resultthresh(:,3)/100-.0001;
        n=parms.NumMatrixRows*parms.NumMatrixColumns;
        br=log2(n)+p.*log2(p)+(1-p).*log2((1-p)/(n-1));
        bx=br/log2(n); % symbols per trial
        exval=2*bx-1;
        
        time=[resultthresh(:,2)]*parms.ISI*(parms.NumMatrixRows+parms.NumMatrixColumns)/60000;
        time=time+(parms.PostSetInterval+parms.PreSetInterval)/60000;
        indx=find(exval<=0);
        exval(indx)=zeros(1,length(indx));
        exval=exval./time;
        subplot(2,1,2); hold on
        plot(resultthresh(:,2),exval,'r','linewidth',2)
        [maxi2,ind]=max(exval);
        if maxi2==0 
            maxi2=1;
        end;
        axis([1 parms.NumberOfSequences 0 max([maxi maxi2])]);
        fprintf('-----------------------------------------\n');
        fprintf('Optimal value of T = %2.2f\n',resultthresh(ind,1));
        fprintf('-----------------------------------------\n');
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


        plotresults(result,handles.MUD,'All MUDs',handles.testfile{rr})

        if parms.NumberOfSequences<size(grandavg,1)
            grandavg(1:minseq,:,rr)=result;
        elseif parms.NumberOfSequences>size(grandavg,1)
            grandavg(:,:,rr)=result(1:minseq,:);
        else
            grandavg(:,:,rr)=result;
        end

    end % each test file

    if rr>1
        figure
        set(gcf,'Name',['All MUDs to All test files'])
        subplot(2,1,1)
        
        gndavg=mean(grandavg,3);
        e=std(grandavg,0,3);
        clr=['b' 'r' 'g' 'c' 'm' 'y' 'k' 'b' 'r' 'g' 'c' 'm' 'y' 'k'];
        for jj=1:size(gndavg,2)
            errorbar(1:minseq,gndavg(:,jj),e(:,jj),clr(jj))
            hold on
        end
        % plot(gndavg,'linewidth',2)
              
        
        set(gca,'YAxisLocation','right')
        axis([1 minseq 0 100])
        title(['All MUDs to All test files'])
        xlabel('# Sequences')
        ylabel('Percent Correct')
        legend(handles.tlabel,'Location','BestOutside')

        subplot(2,1,2)
        ribbon(gndavg)
        axis([.5 handles.MUDindx+.5 1 minseq 0 100])
        view(37.5,30)
        set(gca,'YAxisLocation','right')
        ylabel('# Sequences')
        xlabel('MUD')
        zlabel('Percent Correct')
        legend(handles.tlabel,'Location','BestOutside')

        plotresults(gndavg,handles.MUD,'All MUDs','All test files')
    end
    set(handles.status, 'String', 'Finished applying weights');
end

% --- Executes on button press in saveMUDbutton.
function saveMUDbutton_Callback(hObject, eventdata, handles)
for kk=1:length(handles.nummud)
    if handles.nummud(kk)>handles.MUDindx
    else
        name=[handles.MUDprefix char(handles.tlabel2(handles.nummud(kk)))];
        writeMUD(name,handles.traindatdir,handles.MUD(handles.nummud(kk)));
        set(handles.status, 'String', ['MUD file saved: ' handles.traindatdir name]);
    end
end

% --- Executes on button press in savePRMbutton.
function savePRMbutton_Callback(hObject, eventdata, handles)
% hObject    handle to savePRMbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
for kk=1:length(handles.nummud)
    if handles.nummud(kk)>handles.MUDindx
    else
        name=[handles.MUDprefix char(handles.tlabel2(handles.nummud(kk)))];
        writePRM(name,handles.traindatdir,handles.MUD(handles.nummud(kk)));
        set(handles.status, 'String', ['PRM file saved: ' handles.traindatdir name]);
    end
end

% --- Executes on button press in prmv2.
function prmv2_Callback(hObject, eventdata, handles)
% hObject    handle to prmv2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
for kk=1:length(handles.nummud)
    if handles.nummud(kk)>handles.MUDindx
    else
        name=[handles.MUDprefix char(handles.tlabel2(handles.nummud(kk)))];
        writePRMv2(name,handles.traindatdir,handles.MUD(handles.nummud(kk)));
        set(handles.status, 'String', ['PRM file saved: ' handles.traindatdir name]);
    end
end


% --- Executes on button press in cleartestbutton.
function cleartestbutton_Callback(hObject, eventdata, handles)
handles.testindx=0;
handles = rmfield(handles,'testfile');
handles = rmfield(handles,'testfilesall');
handles = rmfield(handles,'testdirall');
set(handles.applybutton, 'Enable','off');
set(handles.testfiletxt, 'String','No test data selected');
set(handles.cleartestbutton, 'Enable','off');
set(handles.status, 'String', 'Ready');
guidata(hObject,handles)

% --- Executes on button press in clearmudbutton.
function clearmudbutton_Callback(hObject, eventdata, handles)
handles.MUDindx=0;
handles = rmfield(handles,'MUD');
handles=rmfield(handles,'tlabel');
handles=rmfield(handles,'tlabel2');
set(handles.weightlist, 'String', 'No weights loaded')
set(handles.applybutton, 'Enable','off');
set(handles.clearmudbutton, 'Enable','off');
set(handles.saveMUDbutton, 'Enable','off');
set(handles.savePRMbutton, 'Enable','off');
set(handles.prmv2, 'Enable','off');
set(handles.status, 'String', 'Ready');
guidata(hObject,handles)

% --- Executes on button press in chvstimebutton.
function chvstimebutton_Callback(hObject, eventdata, handles)
[signal,state,parms]=getInfo(handles.traindatfiles,handles.traindatdir,[]);
windowlen=round(handles.windlen*parms.SamplingRate/1000);

if handles.SF==3
    signal=CARfilter(signal);
end

signal=single(signal);

SamplingRate=parms.SamplingRate;
SoftwareCh=parms.SoftwareCh;
clear parms
[Responses]=GetP3Responses(signal,state.trialnr,windowlen,state.StimulusCode,state.StimulusType,state.Flashing,1:SoftwareCh,handles.SF,handles.rndsmp);
clear signal
P300chtime(Responses.Responses,Responses.Type,windowlen,SamplingRate,handles.trainfile,handles.SF,handles.stde);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% end buttons
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% --- Executes on button press in useCh2.
function useCh2_Callback(hObject, eventdata, handles)
handles.enableCh2=get(hObject,'Value');
if handles.enableCh2==1
    set(handles.Ch2edit, 'Enable','on');
else
    set(handles.Ch2edit, 'Enable','off');
end
guidata(hObject,handles)

% --- Executes on button press in xlscheck.
function xlscheck_Callback(hObject, eventdata, handles)
handles.xls=get(hObject,'Value');
guidata(hObject,handles)

% --- Executes on selection change in SFpop.
function SFpop_Callback(hObject, eventdata, handles)
handles.SF = get(hObject,'Value');%
guidata(hObject,handles)
% --- Executes during object creation, after setting all properties.
function SFpop_CreateFcn(hObject, eventdata, handles)
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

% --- Executes on selection change in methodpopup.
function methodpopup_Callback(hObject, eventdata, handles)
handles.method = get(hObject,'Value');%
guidata(hObject,handles)

% --- Executes during object creation, after setting all properties.
function methodpopup_CreateFcn(hObject, eventdata, handles)
% hObject    handle to methodpopup (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in stderr.
function stderr_Callback(hObject, eventdata, handles)
handles.stde=get(hObject,'Value');
guidata(hObject,handles)







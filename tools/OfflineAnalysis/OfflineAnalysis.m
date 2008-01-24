%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: OfflineAnalysis.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: OfflineAnalysis.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This file defines a class of functions that control the
%% operation of the OfflineAnalysis GUI and perform data validation.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function varargout = OfflineAnalysis(varargin)
if ~strcmp(computer, 'PCWIN')
  error('BCI2000 Offline Analysis currently works only on windows-based systems.');
  return;
end


% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @OfflineAnalysis_OpeningFcn, ...
                   'gui_OutputFcn',  @OfflineAnalysis_OutputFcn, ...
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

function paramsOut = fillParams(handles, bThourough)
% if bThourough is set to 1, function will confirm that specified files
% exist
settings = get(handles.figTop, 'userdata');
paramsOut = [];

if get(handles.radModeEEG, 'value') == 1
  acqMode = 'eeg';
else
  acqMode = 'ecog';
end
if get(handles.radDomainTime, 'value') == 1
  domain = 'time';
else
  domain = 'freq';
end
params = getAnalysisParams(domain, acqMode, get(handles.lstDataFiles, 'userdata'));
params.montageFile = strtrim(get(handles.txtMontageFile, 'string'));
params.topoParams = strtrim(get(handles.txtTopoFreqs, 'string'));
params.targetConditions{1} = strtrim(get(handles.txtTargetCond1, 'string'));
params.targetConditions{2} = strtrim(get(handles.txtTargetCond2, 'string'));
params.conditionLabels{1} = strtrim(get(handles.txtTargetLabel1, 'string'));
params.conditionLabels{2} = strtrim(get(handles.txtTargetLabel2, 'string'));
params.trialChangeCondition = strtrim(get(handles.txtTrialChangeCond, 'string'));
params.channels = strtrim(get(handles.txtSpecChans, 'string'));
tmp = get(handles.radGrpSpatialFilt, 'SelectedObject');
params.spatialFilt = get(tmp, 'UserData');
  
if strcmp(params.acqType, 'eeg')
  maxFreq = settings.maxFreqEEG;
else
  maxFreq = settings.maxFreqECoG;
end

if bThourough
  for idxFile = 1:length(params.dataFiles)
    if ~exist(params.dataFiles{idxFile}, 'file')
      errordlg(sprintf('Specified data file %s does not exist', ...
        params.dataFiles{idxFile}), 'File not found');
      return;
    end
  end
end
if isempty(params.montageFile) || isempty(params.topoParams)
  %make sure that both are empty
  params.montageFile = '';
  params.topoParams = [];
else
  %user has requested generation of topographies
  if isempty(params.montageFile)
    errordlg('A montage file is required in order to generate topographies.  If you prefer not to generate the topograhies, please uncheck the appropriate box.  Otherwise, please specify a montage file.', 'File not specified')
    return;
  end
  if bThourough && ~exist(params.montageFile, 'file')
    errordlg('Specified montage file does not exist', 'File not found');
    return;
  end
end
if ~isValidStateCondition(params.targetConditions{1}, 'states')
  errordlg('The specified condition for target 1 is invalid', 'Invalid condition');
  return;
end
if isempty(params.conditionLabels{1})
  errordlg('Please specify a value for target label 1', 'Target label 1 not specified');
  return;
end
if ~isValidStateCondition(params.targetConditions{2}, 'states')
  errordlg('The specified condition for target 2 is invalid', 'Invalid condition');
  return;
end
if isempty(params.conditionLabels{2})
  errordlg('Please specify a value for target label 2', 'Target label 2 not specified');
  return;
end
if ~isValidStateCondition(params.trialChangeCondition, 'states')
  errordlg('The specified trial change condition is invalid', 'Invalid trial change condition');
  return;
end
if ~isempty(params.channels)
  bValid = 1;
  try
    params.channels = eval(['[' params.channels ']']);
  catch
    bValid = 0;
  end
  
  if bValid && (...
      ~isnumeric(params.channels) || ...
      any(params.channels ~= floor(params.channels)) || ...
      any(params.channels < 1)...
      )
    bValid = 0;
  end
  if ~bValid
    if get(handles.radDomainTime, 'value')
      errordlg('The specified waveform channel values are invalid', 'Invalid waveform channels');
    else
      errordlg('The specified specturm channel values are invalid', 'Invalid spectrum channels');
    end
    return;
  end
end

if ~isempty(params.montageFile) && ~isempty(params.topoParams)
  %user has requested generation of topographies
  if isempty(params.topoParams)
    if get(handles.radDomainTime, 'value')
      errordlg('At least one topo time is required in order to generate topographies.  If you prefer not to generate the topograhies, please uncheck the appropriate box.  Otherwise, please specify at least one time for analysis.', 'Times required')
    else
      errordlg('At least one topo frequency is required in order to generate topographies.  If you prefer not to generate the topograhies, please uncheck the appropriate box.  Otherwise, please specify at least one frequency for analysis.', 'Frequencies required')
    end
    return;
  else
    bValid = 1;
    try
      params.topoParams = eval(['[' params.topoParams ']']);
    catch
      bValid = 0;
    end
    if bValid
      if get(handles.radDomainTime, 'value') 
        bValid = isnumeric(params.topoParams) && ...
          all(params.topoParams >= 0) && ...
          all(params.topoParams <= settings.dataSegLength);       
      else
        bValid = isnumeric(params.topoParams) && ...
          all(params.topoParams >= -1) && ...
          all(params.topoParams <= maxFreq);        
      end
    end

    if bValid
      if length(params.topoParams) > 9
        if get(handles.radDomainTime, 'value')
          errordlg(sprintf('Maximum number of topo times exceeded.  Please choose up to 9 times from 0 to %d ms.', settings.dataSegLength), 'Number of plots exceeded');
        else
          errordlg(sprintf('Maximum number of topo frequencies exceeded.  Please choose up to 9 frequencies from -1 to %d Hz.', maxFreq), 'Number of plots exceeded');
        end
        return;
      end
    else
      if get(handles.radDomainTime, 'value')
        errordlg(sprintf('The specified topo time values are invalid.  Please choose up to 9 times from 0 to %d ms.', settings.dataSegLength), 'Invalid Topo Frequencies');
      else
        errordlg(sprintf('The specified topo frequency values are invalid.  Please choose up to 9 frequencies from -1 to %d Hz.', maxFreq), 'Invalid Topo Frequencies');
      end
      return;
    end
  end
end

paramsOut = params;
return;


function strout = strtrim(strin)
  strout = regexprep(strin, '^\s*(.*?)\s*$', '$1');
  
% --- Executes just before OfflineAnalysis is made visible.
function OfflineAnalysis_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to OfflineAnalysis (see VARARGIN)

% Choose default command line output for OfflineAnalysis
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes OfflineAnalysis wait for user response (see UIRESUME)
% uiwait(handles.figTop);
try
  settings = load('settings.mat');
catch
  error('Some of the files necessary are missing or have been corrupted.  Please reinstall.');
  return;
end
settings = settings.settings;
set(handles.figTop, 'userdata', settings);


bgColor = [.925 .914 .847];
setBgColors(get(hObject, 'children'), bgColor);

pnlDomain_SelectionChangeFcn(handles.pnlDomain, [], handles);

function setBgColors(objects, bgColor)

for idxObj = 1:length(objects)
  if strcmp(get(objects(idxObj), 'type'), 'uicontrol') && ...
      (strcmp(get(objects(idxObj), 'style'), 'edit') || ...
      strcmp(get(objects(idxObj), 'style'), 'listbox'))
    set(objects(idxObj), 'backgroundcolor', [1 1 1]);
  elseif ~strcmp(get(objects(idxObj), 'type'), 'uimenu') && ~strcmp(get(objects(idxObj), 'tag'), 'lstDataFiles')
    set(objects(idxObj), 'backgroundcolor', bgColor);
    setBgColors(get(objects(idxObj), 'children'), bgColor);
  end
end

% --- Outputs from this function are returned to the command line.
function varargout = OfflineAnalysis_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



function txtDatafile_Callback(hObject, eventdata, handles)
% hObject    handle to txtDatafile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtDatafile as text
%        str2double(get(hObject,'String')) returns contents of txtDatafile as a double


% --- Executes during object creation, after setting all properties.
function txtDatafile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtDatafile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in btnBrowseDataFile.
function btnBrowseDataFile_Callback(hObject, eventdata, handles)
% hObject    handle to btnBrowseDataFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



function txtTargetCond1_Callback(hObject, eventdata, handles)
% hObject    handle to txtTargetCond1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtTargetCond1 as text
%        str2double(get(hObject,'String')) returns contents of txtTargetCond1 as a double


% --- Executes during object creation, after setting all properties.
function txtTargetCond1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtTargetCond1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function txtTargetCond2_Callback(hObject, eventdata, handles)
% hObject    handle to txtTargetCond2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtTargetCond2 as text
%        str2double(get(hObject,'String')) returns contents of txtTargetCond2 as a double


% --- Executes during object creation, after setting all properties.
function txtTargetCond2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtTargetCond2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function txtTrialChangeCond_Callback(hObject, eventdata, handles)
% hObject    handle to txtTrialChangeCond (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtTrialChangeCond as text
%        str2double(get(hObject,'String')) returns contents of txtTrialChangeCond as a double


% --- Executes during object creation, after setting all properties.
function txtTrialChangeCond_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtTrialChangeCond (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function txtSpecChans_Callback(hObject, eventdata, handles)
% hObject    handle to txtSpecChans (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtSpecChans as text
%        str2double(get(hObject,'String')) returns contents of txtSpecChans as a double


% --- Executes during object creation, after setting all properties.
function txtSpecChans_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtSpecChans (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function txtTopoFreqs_Callback(hObject, eventdata, handles)
% hObject    handle to txtTopoFreqs (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtTopoFreqs as text
%        str2double(get(hObject,'String')) returns contents of txtTopoFreqs as a double


% --- Executes during object creation, after setting all properties.
function txtTopoFreqs_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtTopoFreqs (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function txtMontageFile_Callback(hObject, eventdata, handles)
% hObject    handle to txtMontageFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtMontageFile as text
%        str2double(get(hObject,'String')) returns contents of txtMontageFile as a double


% --- Executes during object creation, after setting all properties.
function txtMontageFile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtMontageFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in btnBrowseMontageFile.
function btnBrowseMontageFile_Callback(hObject, eventdata, handles)
% hObject    handle to btnBrowseMontageFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
settings = get(handles.figTop, 'userdata');
if isfield(settings, 'montageFileDir') && ~isempty(settings.montageFileDir)
  [fn path] = uigetfile('*.*', 'Choose a montage file', ...
    settings.montageFileDir);
else
  [fn path] = uigetfile('*.*', 'Choose a montage file');
end

if fn ~= 0
  set(handles.txtMontageFile, 'string', [path fn]);
  settings.montageFileDir = fullfile(path, '.', filesep);
  set(handles.figTop, 'userdata', settings);
end


function txtDataFile_Callback(hObject, eventdata, handles)
% hObject    handle to txtDataFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtDataFile as text
%        str2double(get(hObject,'String')) returns contents of txtDataFile as a double


% --- Executes during object creation, after setting all properties.
function txtDataFile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtDataFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in btnGenPlots.
function btnGenPlots_Callback(hObject, eventdata, handles)
% hObject    handle to btnGenPlots (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
settings = get(handles.figTop, 'userdata');

bDebug = 0;
bSuccess = 0;
plots2Gen = 0;


params = fillParams(handles, 1);


if ~isempty(params)
  if isempty(params.dataFiles)
    errordlg('Please specify at least one data file for analysis.', ...
      'No Data File Specified');
    return;
  end  
  
  plots2Gen = settings.pltSelFeatures;
  if ~isempty(params.montageFile) && ~isempty(params.topoParams)
    plots2Gen = plots2Gen + settings.pltSelTopos;
  end
  if ~isempty(params.channels)
    if strcmp(params.domain, 'time')
      plots2Gen = plots2Gen + settings.pltSelTimeSeq;
    else
      plots2Gen = plots2Gen + settings.pltSelSpectra;
    end
  end

  try

    hWait = waitdlg('Please wait for the analysis to be complete.', 'Please wait');
    %assignin('base', 'params', params);
    if get(handles.chkOverwritePlots, 'value') == 1
      figHandles = get(handles.chkOverwritePlots, 'userdata');
      figHandles = runBasicAnalysis(params, settings, plots2Gen, get(handles.chkIgnoreWarnings, 'value'), 0, figHandles);
    else
      figHandles = runBasicAnalysis(params, settings, plots2Gen, get(handles.chkIgnoreWarnings, 'value'));
    end
    bSuccess = true;

  catch
    [msg msgId] = lasterr;
    msg = regexprep(msg, '.*?\n(.*)', '$1');

    try
      %in case user closed the dialog
      delete(hWait);
    catch
    end


    switch(regexprep(msgId, '.*:(.*)$', '$1'))
      case {'sugTrialsCond1NotMet', 'sugTrialsCond2NotMet'}
        warndlg([msg '  If you wish to continue anyways, please check the box labeled ''Ignore Warnings''.'], 'Warning');
      otherwise
        errordlg(msg, 'Warning');
    end
    return;
  end
  try
    %in case user closed the dialog
    delete(hWait);
  catch
  end

  if strcmpi(get(handles.chkOverwritePlots, 'enable'), 'off')
    set(handles.chkOverwritePlots, 'enable', 'on');
    set(handles.chkOverwritePlots, 'value', 1);
  end
  set(handles.chkOverwritePlots, 'userdata', figHandles);

  topFigPos = get(handles.figTop, 'position');
end

function txtTargetLabel1_Callback(hObject, eventdata, handles)
% hObject    handle to txtTargetLabel1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtTargetLabel1 as text
%        str2double(get(hObject,'String')) returns contents of txtTargetLabel1 as a double


% --- Executes during object creation, after setting all properties.
function txtTargetLabel1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtTargetLabel1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function txtTargetLabel2_Callback(hObject, eventdata, handles)
% hObject    handle to txtTargetLabel2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtTargetLabel2 as text
%        str2double(get(hObject,'String')) returns contents of txtTargetLabel2 as a double


% --- Executes during object creation, after setting all properties.
function txtTargetLabel2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtTargetLabel2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on button press in chkIgnoreWarnings.
function chkIgnoreWarnings_Callback(hObject, eventdata, handles)
% hObject    handle to chkIgnoreWarnings (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkIgnoreWarnings




% --- Executes on button press in chkOverwritePLots.
function chkOverwritePLots_Callback(hObject, eventdata, handles)
% hObject    handle to chkOverwritePLots (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkOverwritePLots


% --- Executes on button press in chkOverwritePlots.
function chkOverwritePlots_Callback(hObject, eventdata, handles)
% hObject    handle to chkOverwritePlots (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkOverwritePlots




% --- Executes on button press in btnRestore.
function btnRestore_Callback(hObject, eventdata, handles)
% hObject    handle to btnRestore (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.pnlRmt, 'visible', 'off');
procPos = get(handles.pnlProc, 'position');
figPos = get(handles.figTop, 'position');
screenDims = get( 0, 'ScreenSize' );
figPos(3) = procPos(3);
if procPos(1)+procPos(3) > screenDims(3)
  proPos(1) = sreenDims(3) - procPos(3);
end
  

% --- Executes on button press in togglebutton6.
function togglebutton6_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton6




% --------------------------------------------------------------------
function btnGrpRmt_SelectionChangeFcn(hObject, eventdata, handles)
% hObject    handle to btnGrpRmt (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

tmp = get(hObject, 'SelectedObject');
figure(get(tmp, 'UserData'));


% --- Executes during object creation, after setting all properties.
function figTop_CreateFcn(hObject, eventdata, handles)
% hObject    handle to figTop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
% figPos = get(hObject, 'position');
% figPos(3) = 108.2;
% figPos(4) = 32.3
% set(hObject, 'position', figPos);



% --------------------------------------------------------------------
function mnuModeEEG_Callback(hObject, eventdata, handles)
% hObject    handle to mnuModeEEG (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(hObject, 'checked', 'on');
set(handles.mnuModeECoG, 'checked', 'off');

% --------------------------------------------------------------------
function mnuModeECoG_Callback(hObject, eventdata, handles)
% hObject    handle to mnuModeECoG (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(hObject, 'checked', 'on');
set(handles.mnuModeEEG, 'checked', 'off');

% --------------------------------------------------------------------
function mnuMode_Callback(hObject, eventdata, handles)
% hObject    handle to mnuMode (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on selection change in lstDataFiles.
function lstDataFiles_Callback(hObject, eventdata, handles)
% hObject    handle to lstDataFiles (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns lstDataFiles contents as cell array
%        contents{get(hObject,'Value')} returns selected item from lstDataFiles
if isempty(get(hObject, 'value'))
  set(hObject, 'value', 1);
end

% --- Executes during object creation, after setting all properties.
function lstDataFiles_CreateFcn(hObject, eventdata, handles)
% hObject    handle to lstDataFiles (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in btnAdd.
function btnAdd_Callback(hObject, eventdata, handles)
% hObject    handle to btnAdd (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
settings = get(handles.figTop, 'userdata');

if isfield(settings, 'dataFileDir') && ~isempty(settings.dataFileDir)
  [fn path] = uigetfile('*.dat', 'Choose a BCI2000 data file', ...
    settings.dataFileDir, 'multiselect', 'on');
else
  [fn path] = uigetfile('*.dat', 'Choose a BCI2000 data file', ...
    'multiselect', 'on');
end

if ~isnumeric(fn)
  if ~iscell(fn)
    fn = {fn};
  end
  
  %save last path as the default
  settings.dataFileDir = fullfile(path, '.', filesep);
  set(handles.figTop, 'userdata', settings);
  
  curFiles = get(handles.lstDataFiles, 'string');
  curFilePaths = get(handles.lstDataFiles, 'userdata');  
  
  for idxFile = 1:length(fn)
    fp = [path fn{idxFile}];
    %makee sure this file isn't already inserted
    if isempty(find(ismember(curFilePaths, fp)==1))
      curFiles{end+1} = fn{idxFile};
      curFilePaths{end+1} = fp;
    end
  end
  set(handles.lstDataFiles, 'string', curFiles);
  set(handles.lstDataFiles, 'userdata', curFilePaths);  
  
  set(handles.btnRemove, 'enable', 'on');
end

% --- Executes on button press in btnRemove.
function btnRemove_Callback(hObject, eventdata, handles)
% hObject    handle to btnRemove (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
curFiles = get(handles.lstDataFiles, 'string');
curFilePaths = get(handles.lstDataFiles, 'userdata');
selectedIdx = get(handles.lstDataFiles, 'value');

curFiles(selectedIdx) = [];
curFilePaths(selectedIdx) = [];

set(handles.lstDataFiles, 'string', curFiles);
set(handles.lstDataFiles, 'userdata', curFilePaths);

if length(curFiles) == 0
  set(handles.btnRemove, 'enable', 'off');
end

if selectedIdx > length(curFiles) && selectedIdx > 1
  set(handles.lstDataFiles, 'value', selectedIdx - 1);
end

% --- Executes on button press in btnClear.
function btnClear_Callback(hObject, eventdata, handles)
% hObject    handle to btnClear (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.lstDataFiles, 'value', 1);
set(handles.lstDataFiles, 'string', {});
set(handles.lstDataFiles, 'userdata', {});
set(handles.btnRemove, 'enable', 'off');



% --- Executes on button press in togglebutton7.
function togglebutton7_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton7




% --- Executes when user attempts to close figTop.
function figTop_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figTop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: delete(hObject) closes the figure
settings = get(handles.figTop, 'userdata');
save 'settings.mat' settings;
delete(hObject);




% --- Executes on button press in chkFeatures.
function chkFeatures_Callback(hObject, eventdata, handles)
% hObject    handle to chkFeatures (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkFeatures



% --- Executes during object creation, after setting all properties.
function btnBrowseMontageFile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to btnBrowseMontageFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called




% --------------------------------------------------------------------
function uipanel12_SelectionChangeFcn(hObject, eventdata, handles)
% hObject    handle to uipanel12 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function pnlDomain_SelectionChangeFcn(hObject, eventdata, handles)
% hObject    handle to pnlMode (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
settings = get(handles.figTop, 'userdata');

if get(handles.radDomainFreq, 'value')
  set(handles.lblChannels, 'string', 'Spectra Channels');
  set(handles.lblTopoParams, 'string', 'Topo Frequencies (Hz)');
  set(handles.txtTargetCond1, 'string', settings.freqCondition1);
  set(handles.txtTargetCond2, 'string', settings.freqCondition2);
  set(handles.txtTargetLabel1, 'string', settings.freqConditionLabel1);
  set(handles.txtTargetLabel2, 'string', settings.freqConditionLabel2);
  set(handles.txtTrialChangeCond, 'string', settings.freqTrialChangeCond);
  set(handles.radSpatialFiltCAR, 'value', 1);
  set(handles.radSpatialFiltNone, 'value', 0);  
else
  set(handles.lblChannels, 'string', 'Waveform Channels');
  set(handles.lblTopoParams, 'string', 'Topo Times (ms)');
  set(handles.txtTargetCond1, 'string', settings.p300Condition1);
  set(handles.txtTargetCond2, 'string', settings.p300Condition2);
  set(handles.txtTargetLabel1, 'string', settings.p300ConditionLabel1);
  set(handles.txtTargetLabel2, 'string', settings.p300ConditionLabel2);
  set(handles.txtTrialChangeCond, 'string', settings.p300TrialChangeCond);
  set(handles.radSpatialFiltCAR, 'value', 0);
  set(handles.radSpatialFiltNone, 'value', 1);  
end


% --------------------------------------------------------------------
function mnuSaveSettings_Callback(hObject, eventdata, handles)
% hObject    handle to mnuSaveSettings (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
params = fillParams(handles, 0);
if ~isempty(params)
  [fn path] = uiputfile('*.bws', 'Save settings as...');
  if fn ~= 0
    try
      save(fullfile(path, fn), 'params');
      msgbox('Current settings have been saved', 'Settings Saved', 'modal');
    catch
      [msg msgId] = lasterr;
      msg = regexprep(msg, '.*?\n(.*)', '$1');    
      errordlg(msg, 'Unable to save', 'modal');
      return;
    end
  end
end

% --------------------------------------------------------------------
function mnuLoadSettings_Callback(hObject, eventdata, handles)
% hObject    handle to mnuLoadSettings (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
[fn path] = uigetfile('*.bws', 'Load settings');
if fn ~= 0
  try
    params = load('-mat', fullfile(path, fn));
  catch
    [msg msgId] = lasterr;
    msg = regexprep(msg, '.*?\n(.*)', '$1');    
    errordlg(msg, 'Invalid file', 'modal');
    return;
  end
  
  if ~isfield(params, 'params')
    errordlg('Specified file is not a valid workspace file.', 'Invalid file', 'modal');
    return;
  end
  
  params = params.params;

  flags = isValidAnalysisParams(params);
  %ignore abscence of data files
  flags = flags - bitand(flags, 1024);
  
  if flags ~= 0
    errordlg('Specified file is not a valid workspace file.', 'Invalid file', 'modal');
    return;
  end

  if strcmp(params.domain, 'time')
    set(handles.radDomainFreq, 'value', 0);
    set(handles.radDomainTime, 'value', 1);
  else
    set(handles.radDomainFreq, 'value', 1);
    set(handles.radDomainTime, 'value', 0);
  end
  pnlDomain_SelectionChangeFcn(handles.pnlDomain, [], handles);
  if strcmp(params.acqType, 'eeg')
    set(handles.radModeECoG, 'value', 0);
    set(handles.radModeEEG, 'value', 1);
  else
    set(handles.radModeECoG, 'value', 1);
    set(handles.radModeEEG, 'value', 0);
  end
  

  dispStr = {};
  for idxFile = 1:length(params.dataFiles)
    [path, fn, ext, versn] = fileparts(params.dataFiles{idxFile});
    dispStr{idxFile} = [fn ext versn];
  end
  set(handles.lstDataFiles, 'value', 1);
  set(handles.lstDataFiles, 'string', dispStr);
  set(handles.lstDataFiles, 'userdata', params.dataFiles);
  if isempty(dispStr)
    set(handles.btnRemove, 'enable', 'off');
  else
    set(handles.btnRemove, 'enable', 'on');
  end

  set(handles.txtMontageFile, 'string', params.montageFile);

  set(handles.txtTargetCond1, 'string', params.targetConditions{1});
  set(handles.txtTargetLabel1, 'string', params.conditionLabels{1});
  set(handles.txtTargetCond2, 'string', params.targetConditions{2});
  set(handles.txtTargetLabel2, 'string', params.conditionLabels{2});

  set(handles.txtTrialChangeCond, 'string', params.trialChangeCondition);
  set(handles.txtSpecChans, 'string', int2str(params.channels));
  set(handles.txtTopoFreqs, 'string', int2str(params.topoParams));

  if strcmpi(params.spatialFilt, 'CAR')
    set(handles.radSpatialFiltCAR, 'value', 1);
    set(handles.radSpatialFiltNone, 'value', 0);
  else
    set(handles.radSpatialFiltCAR, 'value', 0);
    set(handles.radSpatialFiltNone, 'value', 1);
  end
end
    
        
    
% --------------------------------------------------------------------
function mnuFile_Callback(hObject, eventdata, handles)
% hObject    handle to mnuFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)





% --- Executes during object creation, after setting all properties.
function pnlProc_CreateFcn(hObject, eventdata, handles)
% hObject    handle to pnlProc (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called




% --- Executes on button press in radDomainFreq.
function radDomainFreq_Callback(hObject, eventdata, handles)
% hObject    handle to radDomainFreq (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radDomainFreq


% --- Executes during object creation, after setting all properties.
function pnlDomain_CreateFcn(hObject, eventdata, handles)
% hObject    handle to pnlDomain (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called


% --- Executes during object creation, after setting all properties.
function pnlAcqType_CreateFcn(hObject, eventdata, handles)
% hObject    handle to pnlAcqType (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called


% --- Executes on button press in radModeEEG.
function radModeEEG_Callback(hObject, eventdata, handles)
% hObject    handle to radModeEEG (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radModeEEG



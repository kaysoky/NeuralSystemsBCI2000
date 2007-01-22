function varargout = bci_muanalysis_gui(varargin)
% BCI_MUANALYSIS_GUI M-file for bci_muanalysis_gui.fig
%      BCI_MUANALYSIS_GUI, by itself, creates a new BCI_MUANALYSIS_GUI or raises the existing
%      singleton*.
%
%      H = BCI_MUANALYSIS_GUI returns the handle to a new BCI_MUANALYSIS_GUI or the handle to
%      the existing singleton*.
%
%      BCI_MUANALYSIS_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in BCI_MUANALYSIS_GUI.M with the given input arguments.
%
%      BCI_MUANALYSIS_GUI('Property','Value',...) creates a new BCI_MUANALYSIS_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before bci_muanalysis_gui_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to bci_muanalysis_gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help bci_muanalysis_gui

% Last Modified by GUIDE v2.5 22-Jan-2007 15:32:36

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
   'gui_Singleton',  gui_Singleton, ...
   'gui_OpeningFcn', @bci_muanalysis_gui_OpeningFcn, ...
   'gui_OutputFcn',  @bci_muanalysis_gui_OutputFcn, ...
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



% --- Executes just before bci_muanalysis_gui is made visible.
function bci_muanalysis_gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to bci_muanalysis_gui (see VARARGIN)

ImageFileName = 'Mu Rhythm Analysis.png';
try
   [im map] = imread(ImageFileName);
catch
   marioerror('resourcemissing', 'Cannot open: %s', ImageFileName);
end% try
image(im, 'parent', handles.img_axes)
axis off;

% Initialize BCI2k variable, if needed
try
   evalin('base', 'BCI2k;');
catch
BCI2k = struct( ...
   'General',        'empty', ...
   'Files',          'empty', ...
   'Protocol',       'empty', ...
   'Conditioning',   'empty', ...
   'FeatExtraction', 'empty', ...
   'Statistics',     'empty' ...
   )
assignin('base', 'BCI2k', BCI2k);
end% try

% Choose default command line output for bci_muanalysis_gui
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes bci_muanalysis_gui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = bci_muanalysis_gui_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



% --- Executes on button press in DatFile_pushbutton.
function DatFile_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to DatFile_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
BCI2k = evalin('base', 'BCI2k');
try
   DefaultName = BCI2k.Files.FileNames.Dats{1};
catch
   DefaultName = '*.dat';
end% try
%
[DatFileName,DatPathName] = uigetfile(DefaultName,'Select BCI2000 dat files...','MultiSelect', 'on');
if 0 == DatPathName
   return
end% if
nFiles = length(DatFileName);

% convert to cell array even if single dat
if nFiles ==1
   DatFileNames = {DatFileName};
else
   DatFileNames = DatFileName;
end% if

% Fill info into BCI2k structure
BCI2k.Files.FileNames.Dats = {};
for ff = 1:nFiles
   BCI2k.Files.FileNames.Dats{ff} = fullfile(DatPathName, DatFileName{ff});
end% for
% guidata(hObject, handles);
%
% Write BCI2k back into base workspace
assignin('base', 'BCI2k', BCI2k);

% --- Executes on button press in MmfFile_pushbutton.
function MmfFile_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to MmfFile_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
BCI2k = evalin('base', 'BCI2k');
try
   DefaultName = BCI2k.Files.FileNames.Montage;
catch
   DefaultName = '*.mmf';
end% try
%
[MmfFileName,MmfPathName] = uigetfile(DefaultName,'Select BCI2000 mmf file...','MultiSelect', 'off');
if 0 == MmfPathName
   return
end% if
BCI2k.Files.FileNames.Montage = fullfile(MmfPathName, MmfFileName);
% guidata(hObject, handles);
assignin('base', 'BCI2k', BCI2k);


% --- Executes on button press in Open_pushbutton.
function Open_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Open_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.Busy_text, 'Visible', 'on'); drawnow;
BCI2k = evalin('base', 'BCI2k');
[BCI2k.Files, BCI2k.General] = bci_loadfiles( ...
   BCI2k.Files.FileNames.Dats, ...
   BCI2k.Files.FileNames.Montage, ...
   '');
% guidata(hObject, handles);
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)
set(handles.Busy_text, 'Visible', 'off');

% --- Executes on button press in Protocol_pushbutton.
function Protocol_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Protocol_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% --- Executes on button press in ProOpt_pushbutton.
set(handles.Busy_text, 'Visible', 'on'); drawnow;
BCI2k = evalin('base', 'BCI2k');
BCI2k.Protocol = bci_protocol(BCI2k.Files.States, 'CustomAnalysis', BCI2k.Protocol.Definition);
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)
set(handles.Busy_text, 'Visible', 'off');

function ProOpt_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to ProOpt_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
BCI2k = evalin('base', 'BCI2k');
try, BCI2k.Files.States;
catch
   marioerror('earlyevaluation', 'Please open files before setting protocol options!');
   return
end% try
Expressions = struct( ...
   'ValidSamples',  'Feedback == true', ...
   'Regressor'   ,  'TargetCode', ...
   'Break'       ,  '0', ...
   'StimulusCode',  'TargetCode', ...
   'TrialStart'  ,  'risingedge(~TargetCode)' ...
   );
defaultProtOpt = struct(...
   'StateNames', {fieldnames(BCI2k.Files.States)}, ...
   'Expressions', Expressions ...
   );

ProtOpt = bci_protopt_gui(defaultProtOpt);
if ~isstruct(ProtOpt)% User pressed Cancel
   return
end% if
BCI2k.Protocol.Definition = ProtOpt.Expressions;
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)

% --- Executes on button press in SpFilt_pushbutton.
function SpFilt_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to SpFilt_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

BCI2k = evalin('base', 'BCI2k');
% TODO: insert here all available filters, possibly dynamically
% First column: Function name. Second column: Filter Description;
sflFUNC_ =1; sflDESCR_ =2;
SpFiltList = {
   'carspfilt' 'Common Average Reference (CAR)'
   'rawspfilt' 'No Spatial Filtering (raw)'
   };
[SFIndex,ok] = listdlg('ListString', SpFiltList(:, sflDESCR_),...
   'SelectionMode', 'single', ...
   'Name', 'BCI Signal Conditioning', ...
   'PromptString', 'Select spatial filter...' ...
   );
if ~ok, return, end
%
BCI2k.Conditioning.FilterName = SpFiltList{SFIndex, sflFUNC_};
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)

% --- Executes on button press in Conditioning_pushbutton.
function Conditioning_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Conditioning_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.Busy_text, 'Visible', 'on'); drawnow;
BCI2k = evalin('base', 'BCI2k');
BCI2k.Conditioning = bci_conditioning( ...
   BCI2k.Conditioning.FilterName, BCI2k.Files.Data, BCI2k.Files.Geom ...
   );
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)
set(handles.Busy_text, 'Visible', 'off');


% --- Executes on button press in FreqFilt_pushbutton.
function FreqFilt_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to FreqFilt_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in FEOpt_pushbutton.
function FEOpt_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to FEOpt_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
BCI2k = evalin('base', 'BCI2k');
SamplingFrequency = str2double(BCI2k.Files.Parameters.SamplingRate.Value{1});
%
defaultFExParams =  struct( ...
   'EpochOverlap', 0.33, ...
   'EpochLength', 1, ...
   'Hp', 0, ...
   'Lp', 60, ...
   'Step', 0.2, ...
   'BW', 2, ...
   'Trend', 1, ...
   'Order', 16, ...
   'FSamp',  SamplingFrequency ...
   );
ParamNames = fieldnames(defaultFExParams);
ParamValues = struct2cell(defaultFExParams);
nParams = length(ParamNames);
Table.Data = [ParamNames ParamValues];
Table.Columns = {'Parameter', 'Value'};
%
TableData = bci_fetable_gui(Table);
if isempty(TableData), return, end;
% restore datatype
NewParamNames = TableData(:, 1);
NewParamValues = TableData(:, 2);
FExParams = cell2struct(NewParamValues, ParamNames, 1);
% restore datatype
for pp = 1:nParams
   % check if it was edited in the dialog by user
   if ischar(NewParamValues{pp}) && ~ischar(ParamValues{pp})
      FExParams.(ParamNames{pp}) = str2num(FExParams.(ParamNames{pp}));
   end% if
end% for
%
BCI2k.FeatExtraction.Params = FExParams;
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)

% --- Executes on button press in FeatExtr_pushbutton.
function FeatExtr_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to FeatExtr_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.Busy_text, 'Visible', 'on'); drawnow;
BCI2k = evalin('base', 'BCI2k');
% TODO: generalize to other types of epocher and feature extractor
BCI2k.FeatExtraction = bci_featextractor(...
   'iterativemem','muepocher', ...
   BCI2k.FeatExtraction.Params, ...
   BCI2k.Conditioning.FilteredSignal, ...
   BCI2k.Protocol.States);
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)
set(handles.Busy_text, 'Visible', 'off');

% --- Executes on button press in FeatComb_pushbutton.
function FeatComb_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to FeatComb_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.Busy_text, 'Visible', 'on'); drawnow;
set(handles.Busy_text, 'Visible', 'off');


% --- Executes on button press in Stats_pushbutton.
function Stats_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Stats_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.Busy_text, 'Visible', 'on'); drawnow;
BCI2k = evalin('base', 'BCI2k');
BCI2k.Statistics = bci_statanalysis('RSquare',BCI2k.FeatExtraction);
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)
set(handles.Busy_text, 'Visible', 'off');


% --- Executes on button press in Classif_pushbutton.
function Classif_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Classif_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.Busy_text, 'Visible', 'on'); drawnow;
set(handles.Busy_text, 'Visible', 'off');

% --- Executes on button press in StatOpt_pushbutton.
function StatOpt_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to StatOpt_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
BCI2k = evalin('base', 'BCI2k');
% TODO: insert here all available filters, possibly dynamically
% First column: Function name. Second column: Filter Description;
sflFUNC_ =1; sflDESCR_ =2;
StatList = {
   'RSquare' 'r-square analysis'
   };
[SFIndex,ok] = listdlg('ListString', StatList(:, sflDESCR_),...
   'SelectionMode', 'single', ...
   'Name', 'BCI Signal Conditioning', ...
   'PromptString', 'Select spatial filter...' ...
   );
if ~ok, return, end
%
BCI2k.Statistics.Params.Analysis = StatList{SFIndex, sflFUNC_};
assignin('base', 'BCI2k', BCI2k);
disp(BCI2k)



% --- Executes on button press in Show_pushbutton.
function EvalShow_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Show_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% % Open_pushbutton_Callback(hObject, eventdata, handles);
% % Protocol_pushbutton_Callback(hObject, eventdata, handles);
% % Conditioning_pushbutton_Callback(hObject, eventdata, handles);
% % FeatExtr_pushbutton_Callback(hObject, eventdata, handles);
% % % FeatComb_pushbutton_Callback(hObject, eventdata, handles);
% % Stats_pushbutton_Callback(hObject, eventdata, handles);
% % % Classif_pushbutton_Callback(hObject, eventdata, handles);

CommandList = {'Open' 'Protocol' 'Conditioning' 'FeatExtr' 'Stats'};
for module = 1:length(CommandList)
   CurFun = str2func(sprintf('%s_pushbutton_Callback', CommandList{module}));
   try
      CurFun(hObject, eventdata, handles);
   catch
      lasterr = lasterrror;
      marioerror( 'guimodulefailed', ...
         [
         'Execution stopped in module: %s\n' ...
         'possibly due to missing or mistaken parameter setting.\n' ...
         'full description of the error follows:\n%s'
         ], ...
         CommandList(module), lasterr.message);
      return;
   end% try
end% for
%
% Launch visualizer
Show_pushbutton_Callback(hObject, eventdata, handles);


% --- Executes on button press in Show_pushbutton.
function Show_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Show_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
BCI2k = evalin('base', 'BCI2k');
BCI_FeatVisualizer(BCI2k, 'Topo3','TopographViewer','SpectraViewer');
% assignin('base', 'BCI2k', BCI2k);


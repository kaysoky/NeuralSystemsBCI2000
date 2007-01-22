function varargout = bci_protopt_gui(varargin)
% BCI_PROTOPT_GUI M-file for bci_protopt_gui.fig
%      BCI_PROTOPT_GUI, by itself, creates a new BCI_PROTOPT_GUI or raises the existing
%      singleton*.
%
%      H = BCI_PROTOPT_GUI returns the handle to a new BCI_PROTOPT_GUI or the handle to
%      the existing singleton*.
%
%      BCI_PROTOPT_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in BCI_PROTOPT_GUI.M with the given input arguments.
%
%      BCI_PROTOPT_GUI('Property','Value',...) creates a new BCI_PROTOPT_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before bci_protopt_gui_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to bci_protopt_gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help bci_protopt_gui

% Last Modified by GUIDE v2.5 22-Jan-2007 11:10:13

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @bci_protopt_gui_OpeningFcn, ...
                   'gui_OutputFcn',  @bci_protopt_gui_OutputFcn, ...
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


% --- Executes just before bci_protopt_gui is made visible.
function bci_protopt_gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to bci_protopt_gui (see VARARGIN)

% Choose default command line output for bci_protopt_gui
handles.output = nan;

% Fill input argument into handles
if nargin>1 && isstruct(varargin{1}) && isfield(varargin{1}, 'StateNames')
   handles.ProtOpt = varargin{1};
end% if

% Initialize gui controls
BCI2k = evalin('base', 'BCI2k');
set(handles.States_popupmenu, 'String', handles.ProtOpt.StateNames);
VStateNames = fieldnames (handles.ProtOpt.Expressions);
for vs = length(VStateNames)
   VirtualStateName = VStateNames{vs};
   EditName = sprintf('%s_edit', VirtualStateName);
   set(handles.(EditName), 'String', handles.ProtOpt.Expressions.(VirtualStateName));
end% for

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes bci_protopt_gui wait for user response (see UIRESUME)
uiwait(handles.figure1);

% --- Outputs from this function are returned to the command line.
function varargout = bci_protopt_gui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% Get default command line output from handles structure
varargout{1} = handles.output;
% if isstruct(handles.output) || ~isnan(handles.output)% one between Ok or Cancel was pressed
   delete(handles.figure1);
% end% if


function ValidSamples_edit_Callback(hObject, eventdata, handles)
% hObject    handle to ValidSamples_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of ValidSamples_edit as text
%        str2double(get(hObject,'String')) returns contents of ValidSamples_edit as a double


% --- Executes during object creation, after setting all properties.
function ValidSamples_edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to ValidSamples_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Regressor_edit_Callback(hObject, eventdata, handles)
% hObject    handle to Regressor_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Regressor_edit as text
%        str2double(get(hObject,'String')) returns contents of Regressor_edit as a double


% --- Executes during object creation, after setting all properties.
function Regressor_edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Regressor_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit3_Callback(hObject, eventdata, handles)
% hObject    handle to edit3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit3 as text
%        str2double(get(hObject,'String')) returns contents of edit3 as a double


% --- Executes during object creation, after setting all properties.
function edit3_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Stimulus_edit_Callback(hObject, eventdata, handles)
% hObject    handle to Stimulus_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Stimulus_edit as text
%        str2double(get(hObject,'String')) returns contents of Stimulus_edit as a double


% --- Executes during object creation, after setting all properties.
function Stimulus_edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Stimulus_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function TrialStart_edit_Callback(hObject, eventdata, handles)
% hObject    handle to TrialStart_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of TrialStart_edit as text
%        str2double(get(hObject,'String')) returns contents of TrialStart_edit as a double


% --- Executes during object creation, after setting all properties.
function TrialStart_edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to TrialStart_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in Ok_pushbutton.
function Ok_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Ok_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
VStateNames = fieldnames (handles.ProtOpt.Expressions);
for vs = length(VStateNames)
   VirtualStateName = VStateNames{vs};
   EditName = sprintf('%s_edit', VirtualStateName);
   handles.ProtOpt.Expressions.(VirtualStateName) = get(handles.(EditName), 'String');
end% for
handles.output = handles.ProtOpt;
guidata(hObject, handles);
uiresume(handles.figure1);

% --- Executes on button press in Cancel_pushbutton.
function Cancel_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Cancel_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.output = false;
guidata(hObject, handles);
uiresume(handles.figure1);


% --- Executes on selection change in States_popupmenu.
function States_popupmenu_Callback(hObject, eventdata, handles)
% hObject    handle to States_popupmenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns States_popupmenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from States_popupmenu
StateNames = get(hObject, 'String');
StateName = StateNames{get(hObject, 'Value')};
clipboard('copy', StateName);

% --- Executes during object creation, after setting all properties.
function States_popupmenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to States_popupmenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end





function Break_edit_Callback(hObject, eventdata, handles)
% hObject    handle to Break_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Break_edit as text
%        str2double(get(hObject,'String')) returns contents of Break_edit as a double


% --- Executes during object creation, after setting all properties.
function Break_edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Break_edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




% --- Executes during object creation, after setting all properties.
function figure1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called


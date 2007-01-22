function varargout = bci_fetable_gui(varargin)
% BCI_FETABLE_GUI M-file for bci_fetable_gui.fig
%      BCI_FETABLE_GUI, by itself, creates a new BCI_FETABLE_GUI or raises the existing
%      singleton*.
%
%      H = BCI_FETABLE_GUI returns the handle to a new BCI_FETABLE_GUI or the handle to
%      the existing singleton*.
%
%      BCI_FETABLE_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in BCI_FETABLE_GUI.M with the given input arguments.
%
%      BCI_FETABLE_GUI('Property','Value',...) creates a new BCI_FETABLE_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before bci_fetable_gui_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to bci_fetable_gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help bci_fetable_gui

% Last Modified by GUIDE v2.5 22-Jan-2007 14:21:59

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @bci_fetable_gui_OpeningFcn, ...
                   'gui_OutputFcn',  @bci_fetable_gui_OutputFcn, ...
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


% --- Executes just before bci_fetable_gui is made visible.
function bci_fetable_gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to bci_fetable_gui (see VARARGIN)
Table = varargin{1};
 handles.Prm_table = uitable( ...
   Table.Data, Table.Columns, ...
   ... 'Parent', handles.figure1);
   'Parent', handles.FE_uipanel);

set(handles.Prm_table, 'Units', 'normalized');
set(handles.Prm_table, 'Position', [.07 .12 .86 .76]);
set(handles.Prm_table, 'Editable', true);

% Choose default command line output for bci_fetable_gui
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes bci_fetable_gui wait for user response (see UIRESUME)
uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = bci_fetable_gui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
delete(handles.figure1)


% --- Executes on button press in Cancel_pushbutton.
function Cancel_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Cancel_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.output = [];
guidata(hObject, handles);
uiresume(handles.figure1);


% --- Executes on button press in Ok_pushbutton.
function Ok_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Ok_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
TableData = get(handles.Prm_table, 'Data');
handles.output = cell(TableData);
guidata(hObject, handles);
uiresume(handles.figure1);




% --- Executes when user attempts to close figure1.
function figure1_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: delete(hObject) closes the figure
uiresume(hObject);



function varargout = BCI_FeatVisualizer(varargin)
% BCI_FEATVISUALIZER M-file for BCI_FeatVisualizer.fig
%      BCI_FEATVISUALIZER, by itself, creates a new BCI_FEATVISUALIZER or raises the existing
%      singleton*.
%
%      H = BCI_FEATVISUALIZER returns the handle to a new BCI_FEATVISUALIZER or the handle to
%      the existing singleton*.
%
%      BCI_FEATVISUALIZER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in BCI_FEATVISUALIZER.M with the given input arguments.
%
%      BCI_FEATVISUALIZER('Property','Value',...) creates a new BCI_FEATVISUALIZER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before BCI_FeatVisualizer_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to BCI_FeatVisualizer_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help BCI_FeatVisualizer

% Last Modified by GUIDE v2.5 22-Jan-2007 18:35:41

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @BCI_FeatVisualizer_OpeningFcn, ...
                   'gui_OutputFcn',  @BCI_FeatVisualizer_OutputFcn, ...
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


% --- Executes just before BCI_FeatVisualizer is made visible.
function BCI_FeatVisualizer_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to BCI_FeatVisualizer (see VARARGIN)

% Choose default command line output for BCI_FeatVisualizer
handles.output = hObject;

handles.BCI2k = varargin{1};
handles.LeftEvent = varargin{2};
handles.MiddleEvent = varargin{3};
handles.RightEvent = varargin{4};

axes(handles.axes1);
handles.BCI2k.Visualizer = flat_BCI_FeatVisualizer(varargin{:});

%----ColorMap----
x = get(handles.popupmenu1,'string');
SelectedCMapMask = strfind(x,handles.BCI2k.Visualizer.FeatVisualizer.Colormap);
for i = 1:length(SelectedCMapMask)
    if ~isempty(SelectedCMapMask{i})
        SelectedCMap = i;
    end
end
set(handles.popupmenu1,'value',SelectedCMap);
%----------------

%----ColorScaling----
CMin = handles.BCI2k.Visualizer.FeatVisualizer.ColorScaling.Min;
set(handles.edit1,'string',sprintf('%.3f',CMin));
CMax = handles.BCI2k.Visualizer.FeatVisualizer.ColorScaling.Max;
set(handles.edit2,'string',sprintf('%.3f',CMax));
%----------------

%----Shading----
xx = get(handles.popupmenu2,'string');
SelectedShadingMap = strfind(xx,handles.BCI2k.Visualizer.FeatVisualizer.Shading);
for i = 1:length(SelectedShadingMap)
    if ~isempty(SelectedShadingMap{i})
        SelectedShading = i;
    end
end
set(handles.popupmenu2,'value',SelectedShading);
%----------------

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes BCI_FeatVisualizer wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = BCI_FeatVisualizer_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
% varargout{1} = handles.output;


% --- Executes on slider movement.
function slider1_Callback(hObject, eventdata, handles)
% hObject    handle to slider1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider


% --- Executes during object creation, after setting all properties.
function slider1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to slider1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes on button press in pushbutton1.
function pushbutton1_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%----ColorMap----
ColorMaps = get(handles.popupmenu1,'string');
SelColorMapPos = get(handles.popupmenu1,'value');
handles.BCI2k.Visualizer.FeatVisualizer.Colormap = ColorMaps{SelColorMapPos};
%----------------

%----ColorScaling----
CMin = str2double(get(handles.edit1,'string'));
CMax = str2double(get(handles.edit2,'string'));
handles.BCI2k.Visualizer.FeatVisualizer.ColorScaling.Min = CMin;
handles.BCI2k.Visualizer.FeatVisualizer.ColorScaling.Max = CMax;
%----------------

%----Shading----
ShadingMaps = get(handles.popupmenu2,'string');
SelShadingMapPos = get(handles.popupmenu2,'value');
handles.BCI2k.Visualizer.FeatVisualizer.Shading = ShadingMaps{SelShadingMapPos};
%----------------

axes(handles.axes1);
handles.BCI2k.Visualizer = flat_BCI_FeatVisualizer(handles.BCI2k,'TopographViewer','ComboViewer','SpectraViewer');

% Update handles structure
guidata(hObject, handles);


% --- Executes on selection change in popupmenu1.
function popupmenu1_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu1 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu1

% --- Executes during object creation, after setting all properties.
function popupmenu1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit1_Callback(hObject, eventdata, handles)
% hObject    handle to edit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit1 as text
%        str2double(get(hObject,'String')) returns contents of edit1 as a double


% --- Executes during object creation, after setting all properties.
function edit1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit2_Callback(hObject, eventdata, handles)
% hObject    handle to edit2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit2 as text
%        str2double(get(hObject,'String')) returns contents of edit2 as a double


% --- Executes during object creation, after setting all properties.
function edit2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton2.
function pushbutton2_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
figure;
flat_BCI_FeatVisualizer(handles.BCI2k,'TopographViewer','ComboViewer','SpectraViewer');



% --- Executes on selection change in popupmenu2.
function popupmenu2_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu2 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu2


% --- Executes during object creation, after setting all properties.
function popupmenu2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton4.
function pushbutton4_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.BCI2k.Visualizer = rmfield(handles.BCI2k.Visualizer,'FeatVisualizer');
axes(handles.axes1);
handles.BCI2k.Visualizer = flat_BCI_FeatVisualizer(handles.BCI2k,'TopographViewer','ComboViewer','SpectraViewer');

%----ColorMap----
x = get(handles.popupmenu1,'string');
SelectedCMapMask = strfind(x,handles.BCI2k.Visualizer.FeatVisualizer.Colormap);
for i = 1:length(SelectedCMapMask)
    if ~isempty(SelectedCMapMask{i})
        SelectedCMap = i;
    end
end
set(handles.popupmenu1,'value',SelectedCMap);
%----------------

%----ColorScaling----
CMin = handles.BCI2k.Visualizer.FeatVisualizer.ColorScaling.Min;
set(handles.edit1,'string',sprintf('%.3f',CMin));
CMax = handles.BCI2k.Visualizer.FeatVisualizer.ColorScaling.Max;
set(handles.edit2,'string',sprintf('%.3f',CMax));
%----------------

% Update handles structure
guidata(hObject, handles);


% --- Executes on mouse press over axes background.
function axes1_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to axes1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)




% --- Executes on mouse press over figure background.
function figure1_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



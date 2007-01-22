function Conditioning = bci_conditioning(SpatFilt, Data, Geom)
% BCI_CONDITIONING

% TODO: revise all parameter forwarding

%% Spatial Filtering setup
FunctionString = sprintf('bci_%s', lower(SpatFilt));
if ~exist(FunctionString, 'file')% looks for an m-file with the same name of the filter
   marioerror('nonexistentfunction', 'Spatial Filter not found %s', FunctionString);
end% if
SpatFiltFunc = str2func(FunctionString);
FilteringData = SpatFiltFunc(Data, Geom);
Conditioning.Geom = FilteringData.Geom;

%% Frequency Filtering setup
% TODO:


%% Application of filters to data
% TODO: create shortcut for raw signals
Conditioning.FilteredSignal = double(Data)*FilteringData.SpatFiltMatrix;
Conditioning.FilterName = SpatFilt;
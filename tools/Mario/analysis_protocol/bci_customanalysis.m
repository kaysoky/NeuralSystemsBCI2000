function VirtualStates = bci_customanalysis(States, varargin)
% BCI_CUSTOMANALYSIS implement custom analysis protocol
% TODO: help, copyright, licence
%
% Functions and variables available in custom expression
%     - all states in the dat file
%     - any Matlab command
%     - any custom function available in the path
%     - a number of specific functions defined in this module:
%        - edge(state, delta)
%        - risingedge(state)
%        - fallingedge(state)
%        - shift(state, nsamples)
%        - ...
%
% Authors: A. Emanuele Fiorilla, Febo Cincotti

%% Initialization
State_Names = fieldnames(States);
StateLength = length(States.(State_Names{1}));

Expressions = varargin{1};

VirtualStatesNames = {'ValidSamples' 'Regressor' 'Break' 'StimulusCode' 'TrialStart'};
nVirtualStates = length(VirtualStatesNames);

%% Copy recording states into the current workspace
% This allows an easy way to evaluate custom expressions with no explicit
% parsing
for i = 1:length(State_Names)
   assignment = sprintf('%s = States.%s;', State_Names{i}, State_Names{i});
   eval(assignment);
end

%% Evaluate custom assignments
for vv = 1:nVirtualStates
   VirtualStatesName = VirtualStatesNames{vv};
   %
   try
      VirtualStates.(VirtualStatesName) = eval([Expressions.(VirtualStatesName),';']);
   catch
      lasterr = lasterror;
      marioerror('flawedexpression', ...
         'Cannot evaluate custom expression for state %s:\n%s', ...
         VirtualStatesName, lasterr.message);
   end% try
   % Incomplete expressions are allowed, resulting in empty or scalar
   % states
   %    Fill if empty
   if isempty(VirtualStates.(VirtualStatesName))
      VirtualStates.(VirtualStatesName) = zeros([StateLength 1]);
   end% if
   %    Expand scalar states
   if isscalar(VirtualStates.(VirtualStatesName))
      VirtualStates.(VirtualStatesName) = repmat(VirtualStates.(VirtualStatesName), [StateLength 1]);
   end% if
end

%% Internal functions --------------------------------------
function eventstate = edge(state, delta)
try, thr; catch, thr = eps; end;
if delta>0
   eventstate = [false; diff(state)>delta];
elseif delta<0
   eventstate = [false; diff(state)<delta];
else
   eventstate = false(size(state));
end% if

function eventstate = risingedge(state)
eventstate = logical([0; diff(state)>0]);

function eventstate = fallingedge(state)
eventstate = logical([0; diff(state)<0]);

function eventstate = shift(state, nsamp)
if nsamp>=0
   eventstate = [ zeros([nsamp 1]); state(1:end-nsamp) ];
else
   eventstate = [ state(nsamp+1:end); zeros([nsamp 1]) ];
end% if   

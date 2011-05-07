function [out, err] = bci2000chain(datfile, chain, varargin)
% BCI2000CHAIN    Run a BCI2000 data file through a chain of command-line filters
% 
% S = BCI2000CHAIN(DATFILE, CHAIN, ...)
% 
% DATFILE is the name of, or path to, a BCI2000 data file.
% 
% CHAIN   is a specification of the chain of command-line filters to run
%         through. It can be a cell array of strings, e.g.
%                       {'TransmissionFilter', 'SpatialFilter', 'ARFilter'}
%         or a pipe-delimited string, e.g.
%                               'TransmissionFilter|SpatialFilter|ARFilter'
%         The strings 'ARSignalProcessing' or 'P3SignalProcessing' may be
%         used as a shorthand for the complete pipe for those particular
%         signal processing modules. 
% 
% By default, the chain uses the parameters that were stored in the data
% file. However, additional optional arguments (...) are passed to
% MAKE_BCIPRM, thereby allowing you the option of changing parameter
% values.
% 
% Example:
%     fn = bci2000path('data', 'samplefiles', 'eeg3_2.dat');
%     s = bci2000chain(fn, 'TransmissionFilter|SpatialFilter|ARFilter', ...
%                          'ExampleParameters.prm', 'SpatialFilterType', 3)
% 
% This example reads and replays the sample data file through the specified
% filter chain, after substituting in the parameters specified in
% ExampleParameters.prm, and switching the spatial filter to CAR mode.
% 
% BCI2000CHAIN has the following dependencies:
% 
% M-files:   make_bciprm, read_bcidate, read_bciprm
% Mex-files: load_bcidat, convert_bciprm  (used by the m-files above)
% Binaries:  bci_dat2stream, bci_stream2mat
%            Additional filter binaries (e.g. TransmissionFilter, etc.)
%            The location of these binaries must be added to the PATH
%            environment variable. The optional utility bci2000path.m is
%            useful for this.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id$ 
%% Author: Jeremy Hill <jezhill@gmail.com>
%%
%% $BEGIN_BCI2000_LICENSE$
%% 
%% This file is part of BCI2000, a platform for real-time bio-signal research.
%% [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
%% 
%% BCI2000 is free software: you can redistribute it and/or modify it under the
%% terms of the GNU General Public License as published by the Free Software
%% Foundation, either version 3 of the License, or (at your option) any later
%% version.
%% 
%% BCI2000 is distributed in the hope that it will be useful, but
%%                         WITHOUT ANY WARRANTY
%% - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
%% A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
%% 
%% You should have received a copy of the GNU General Public License along with
%% this program.  If not, see <http://www.gnu.org/licenses/>.
%% 
%% $END_BCI2000_LICENSE$
%% http://www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


if nargin < 2, chain = ''; end
if isempty(chain), chain = 'ARSignalProcessing'; end
if isequal(chain, 'ARSignalProcessing')
	chain = 'TransmissionFilter|SpatialFilter|ARFilter|LinearClassifier|LPFilter|ExpressionFilter|Normalizer';
end
if isequal(chain, 'P3SignalProcessing')
	chain = 'TransmissionFilter|SpatialFilter|P3TemporalFilter|LinearClassifier';
end
if ischar(chain)
	cc = {}; while ~isempty(chain), [cc{end+1,1}, chain] = strtok(chain, '|'); end
	chain = cc;
end
rm = logical(zeros(size(chain)));
for i = 1:numel(chain)
	chain{i} = deblank(fliplr(deblank(fliplr(chain{i}))));
	rm(i) = isempty(chain{i});
end
chain(rm) = [];
if isempty(char(chain)), error('chain is empty'), end

opts = {};
prm = {};
for i = 1:numel(varargin)
	if ischar(varargin{i})
		if strncmp(varargin{i}, '-', 1)
			opts{end+1} = varargin{i};
			continue
		end
	end
	prm{end+1} = varargin{i};
end
verbose = ismember('-v', opts);
if ismember('-p', opts), pretty = {'-p'}; else pretty = {}; end

cmd = {};
tmpdir = tempname;
if verbose, fprintf('creating directory %s\n', tmpdir); end
[success, msg] = mkdir(tmpdir);
if ~success, error(msg), end

prmfile_in  = fullfile(tmpdir, 'in.prm');
prmfile_out = fullfile(tmpdir, 'out.prm');
matfile     = fullfile(tmpdir, 'out.mat');
bcifile     = fullfile(tmpdir, 'out.bci');

if isunix
	DYLD_LIBRARY_PATH = getenv('DYLD_LIBRARY_PATH');
	DYLD_FRAMEWORK_PATH = getenv('DYLD_FRAMEWORK_PATH');
	setenv('DYLD_LIBRARY_PATH', '');  setenv('DYLD_FRAMEWORK_PATH', '')
	% this seems to be required if any of the command-line filters have been built with a QT dependency
	% causing them to be dependent on the Qt framework dylibs, which for some reason cannot be accessed
	% because of Matlab's setting of these two environment variables.  They are set back the way they
	% were below.
end

if ~exist(datfile, 'file') | isdir(datfile), error(sprintf('file %s not found', datfile)), end
if ~isempty(which(datfile)), datfile = which(datfile); end
datfile = resolve(datfile);

mappings = {
	'$DATFILE'     datfile
	'$PRMFILE_IN'  prmfile_in
	'$PRMFILE_OUT' prmfile_out
	'$MATFILE'     matfile
	'$BCIFILE'     bcifile
};

if isempty(prm)
	cmd{end+1} = 'bci_dat2stream < "$DATFILE"';
else
	if ~iscell(prm), prm = {prm}; end
	make_bciprm(datfile, prm{:}, pretty{:}, '>', prmfile_in);
	
	%cmd{end+1} = '(';   % old-style bci_dat2stream with no -p option
	%cmd{end+1} = '   bci_prm2stream < $PRMFILE_IN';
	%cmd{end+1} = '&& bci_dat2stream --transmit-sd < $DATFILE';
	%cmd{end+1} = ')';
	
	cmd{end+1} = 'bci_dat2stream -p$PRMFILE_IN < "$DATFILE"'; % new-style bci_dat2stream with -p option
end
for i = 1:numel(chain), cmd{end+1} = sprintf('| %s', chain{i}); end
% cmd{end+1} = ' > $BCIFILE && bci_stream2mat < $BCIFILE > $MATFILE && bci_stream2prm < $BCIFILE > $PRMFILE_OUT';  % old-style bci_stream2mat without Parms output
cmd{end+1} = '| bci_stream2mat > $MATFILE'; % new-style bci_stream2mat with Parms output

cmd = cmd(:)'; cmd(2, :) = {' '}; cmd = [cmd{1:end-1}];

for i = 1:size(mappings, 1), cmd = strrep(cmd, mappings{i, :}); end
if verbose, fprintf('%s\n', cmd); end

t = clock;
[failed output] = system(cmd);
chaintime = etime(clock, t);
output = deblank(output);
output = fliplr(deblank(fliplr(output)));
output = strrep(output, char([13 10]), char(10));
output = strrep(output, char([13]), char(10));
failsig = 'Configuration Error: ';
failmatch = findstr([failsig output], failsig);
if length(failmatch) > 1, failed = 1; end % TODO: really SYSTEM should have caught this. Is this Windoze-specific?
if verbose & (nargout>=2 | ~failed), fprintf('%s\n', output); end
if failed
	if verbose
		err = sprintf('system call failed:\n%s', output); % cmd has already been printed, so don't clutter things further
	else
		err = sprintf('system call failed:\n%s\n%s', cmd, output);
	end
else
	err = '';
end


if isunix
	setenv('DYLD_LIBRARY_PATH', DYLD_LIBRARY_PATH) % see above
	setenv('DYLD_FRAMEWORK_PATH', DYLD_FRAMEWORK_PATH)
end

if isempty(err)
	mat = load(matfile);
	if ~isfield(mat, 'Data'), err = sprintf('chain must have failed: no ''Data'' variable found in %s', matfile); end
	if ~isfield(mat, 'Index'), err = sprintf('chain must have failed: no ''Index'' variable found in %s', matfile); end
end

if isempty(err)
	out.FileName = datfile;
	if isfield(mat, 'Parms')  % should be there in the file, if your bci_stream2mat is up to date and we're using the new style (single chain)
		parms = read_bciprm(mat.Parms);
	else
		parms = read_bciprm(prmfile_out); % if you get an error that prmfile_out does not exist, either recompile your bci_dat2stream and bci_stream2mat binaries from up-to-date sources, or uncomment the calls to the old style, above 
	end
	out.DateStr = read_bcidate(parms, 'ISO');
	out.DateNum = read_bcidate(parms);
	out.FilterChain = chain(:)';
	out.ShellInput = cmd;
	out.ShellOutput = output;
	out.ChainTime = chaintime;
	out.ChainSpeedFactor = nan;
	out.Megabytes = nan;
	out.Parms = parms;
	sigind = mat.Index.Signal;  % indices vary across channels fastest, then elements
	[nChannels nElements] = size(sigind);
	nBlocks = size(mat.Data, 2);
	out.Blocks = nBlocks;
	out.BlocksPerSecond = parms.SamplingRate.NumericValue  / parms.SampleBlockSize.NumericValue;
	out.SecondsPerBlock = parms.SampleBlockSize.NumericValue  / parms.SamplingRate.NumericValue;
	out.ChainSpeedFactor = out.Blocks * out.SecondsPerBlock / out.ChainTime;
	out.Channels = nChannels;
	if isfield(mat, 'ChannelLabels'), out.ChannelLabels = mat.ChannelLabels(:)'; end
	out.Elements = nElements;
	if isfield(mat, 'ElementLabels'), out.ElementLabels = mat.ElementLabels(:)'; end
	if isfield(mat, 'ElementValues'), out.ElementValues = mat.ElementValues(:)'; end
	if isfield(mat, 'ElementUnit'), out.ElementUnit = mat.ElementUnit; end
	
	out.Time = out.SecondsPerBlock * single(0:nBlocks-1)';
	out.Signal = mat.Data(sigind(:), :);  % nChannels*nElements - by - nBlocks

	switch chain{end}
		case {'ARFilter', 'P3TemporalFilter'} % 3-dimensional output makes more sense than continuous 2-D: "elements" can't just be concatenated into an unbroken time-stream
			out.Signal = reshape(out.Signal, [nChannels nElements nBlocks]); % nChannels - by - nElements - by - nBlocks
			out.Signal = permute(out.Signal, [3 1 2]);                       % nBlocks - by - nChannels - by - nElements
		otherwise
			out.Signal = reshape(out.Signal, [nChannels nElements*nBlocks]); % nChannels - by - nSamples
			out.Signal = permute(out.Signal, [2 1]);                         % nSamples - by - nChannels
	end
	out.States = rmfield(mat.Index, 'Signal');
	statenames = fieldnames(out.States);
	for i = 1:numel(statenames)
		out.States.(statenames{i}) = mat.Data(out.States.(statenames{i}), :)';
	end
	out.Megabytes = getfield(whos('out'), 'bytes') / 1024^2;
	if exist(prmfile_in,  'file'), delete(prmfile_in),  end
	if exist(prmfile_out, 'file'), delete(prmfile_out), end
	if exist(bcifile,     'file'), delete(bcifile),     end
	if exist(matfile,     'file'), delete(matfile),     end
	if exist(tmpdir,      'dir'),  rmdir(tmpdir),       end
else
	out = [];
end

if nargout < 2, error(err), end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function p = resolve(p)

if isdir(p)
	olddir = pwd; cd(p); p = pwd; cd(olddir)
else
	[pdir stem xtn] = fileparts(p);
	if ~isempty(pdir)
		if isdir(pdir)
			pdir = resolve(pdir);
		end
		p = fullfile(pdir, [stem xtn]);
	end
end

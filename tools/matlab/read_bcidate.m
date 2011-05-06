function d = read_bcidate(d, fmt)
% READ_BCIDATE(X)
%     X is anything that READ_BCIPRM can cope with (usually and most
%     meaningfully a .dat filename). Returns a Matlab NOW-style date
%     number.
% 
% READ_BCIDATE(X, FMT)
%     Returns a date-stamp formatted as a string according to the
%     DATESTR-style string FMT.
% 
% READ_BCIDATE(X, 'ISO')
%     Is a shorthand for READ_BCIDATE(X, 'yyyy-mm-dd HH:MM:SS')
% 

d = wrangle(d);
if any(isnan(d(:))), error('failed to unpack a recognized date format from the input'), end
if nargin >= 2
	if isequal(lower(fmt), 'iso'), fmt = 'yyyy-mm-dd HH:MM:SS'; end
	d = datestr(d, fmt);
end


function d = wrangle(d)
fmt1 = 'mmm dd HH:MM:SS yyyy';  % format for v.2 and earlier (must remove 5 characters at start for day of week)
fmt2 = 'yyyy-mm-ddTHH:MM:SS';   % format for v.3 and later
if ischar(d)
	try, d = datenum(d(5:end), fmt1); return, end  % convert string directly to datenum via format 1 if possible
	try, d = datenum(d(1:end), fmt2); return, end  % convert string directly to datenum via format 2 if possible
	% if it wasn't either of those two date formats, assume it's a .dat
	% filename, a .prm filename, or prm content. READ_BCIPRM handles these
	% and returns them as a struct of type 2  (fieldnames = parameter names)
	d = read_bciprm(d);
elseif numel(d) ~= 1 & ~iscell(d)
	% a multiple-element struct or numeric array will have to be dealt with
	% element-by-element, so convert it to a cell array
	d = num2cell(d);
end

if isa(d, 'bcistream'), d = getparam(d); end
% don't worry about this: for systems on which the bcistream class is installed, there will be a getparam method; for others this condition will not be triggered at all

% could be a struct of type (1), which has the type-2 parameter struct in subfield 'Parms' (e.g. the output of BCI2000CHAIN)
if isstruct(d), if isfield(d, 'Parms'), d = d.Parms; end, end

% could be a struct of type (2), like the output of READ_BCIPRM (each subfield contains a parameter, either a cell array of strings or as a type-3 struct, depending on your convert_bciprm mex version)
if isstruct(d), if isfield(d, 'StorageTime'), d = d.StorageTime; end, end

% could be a struct of type (3), with fields 'Type', 'Value', 'DefaultValue', etc, like the substructures of the output of later versions of the CONVERT_BCIPRM mexfile
if isstruct(d), if isfield(d, 'Value'), d = d.Value; end, end

% if it's a cell array, recursively call this function on each cell and stack the results together in a numeric array
if iscell(d)
	for i = 1:numel(d), d{i} = read_bcidate(d{i}); end
	d = cell2mat(d);
	return
end

% by now, D has either been unpacked as far as being a single string that we can parse,
% or an entity we can convert to a datenum some other way,
% or something rogue that we're entirely unable to parse

if ischar(d)
	try, d = datenum(d(5:end), fmt1); return, end
	try, d = datenum(d(1:end), fmt2); return, end
else
	try, d = datenum(d); return, end
end
d = nan; % datestamp FAIL

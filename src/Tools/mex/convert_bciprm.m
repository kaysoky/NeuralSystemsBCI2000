result = convert_bciprm( varargin )
%CONVERT_BCIPRM Convert between Matlab representations of BCI2000 parameters.
%
% A Matlab (mex) subroutine that converts BCI2000 parameters
% from Matlab struct into string representation and back.
%
%  parameter_lines = convert_bciprm( parameter_struct );
%
% converts a BCI2000 parameter struct (as created by load_bcidat)
% into a cell array of strings containing valid BCI2000 parameter
% definition strings.
%
% When the input is a cell array rather than a Matlab struct, convert_bciprm
% will interpret the input as a list of BCI2000 parameter definition strings,
% and convert them into a parameter struct (as returned by load_bcidat):
%
%  parameter_struct = convert_bciprm( parameter_lines );
%

%  This is a help file documenting the functionality contained in
%  convert_bciprm.mex.
%  $Id$
error( 'There is no convert_bciprm mex file for your platform available.' );

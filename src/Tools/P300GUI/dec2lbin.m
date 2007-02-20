function s=dec2lbin(d,n)
%DEC2LBIN Convert decimal integer to a binary string.
%   DEC2LBIN(D,N) returns the binary representation of D with at least N bits 
%   as a vector of logicals.
%   D must be a non-negative integer smaller than 2^52.
%   If D is a matrix, this function will return a logical matrix in which
%   each element has been substituted by an N-et of logicals (thus same
%   number of rows and N times the number of original columns)
%
%   Example
%      dec2lbin(23, 5) returns [1 0 1 1 1]
%      dec2lbin([2 3; 0 1], 2) returns [1 0 1 1; 0 0 0 1]
%
%   This function was developed to serve in a tight loop, so no parameter
%   check is performed for the sake of efficiency. You were warned ;)
%
%   See also DEC2BIN, BIN2DEC, DEC2HEX, DEC2BASE.

%   Developed after Hans Olsson's BIN2DEC
%   Febo Cincotti, f.cincotti@hsantalucia.it
%   $Revision$  $Date$
%   (C) 2000-2007, BCI2000 Project
%   http://www.bci2000.org

[nrows ncols]=size(d);
dt=d';
s_veclogical=rem(floor(dt(:)*pow2((1-n):0)),2);
s=reshape(s_veclogical', [n*ncols nrows])';

function [ressq, sgnR2] = calc_rsqu_f(NDependentVariables,IndependentVariable)
% function [ressq, sgnR2] = calc_rsqu_f(datastruct)
%RSQU   [ressq, avgamp1, avgamp2]  = calc_rsqu(data1, data2, rorrsqu) calculates the r2-value for
%       two three-dimensional variables (dim1 by dim2 by trial) data1 and data2
%       the result is ressq (dim1, dim2); each element represents the r2 
%       for this particular combination of dim1, dim2 across trials
%       in addition to r2 values, this function also calculates average amplitudes 
%       for each sample and channel, for both data variables (i.e., conditions), and
%       returns these in amp1 and amp2
%       rorrsqu == 1 ... rsqu values
%                  2 ... r values
%       if rorrsq is not provided (i.e., function is called with two
%       parameters only, it defaults to rsqu values)


% [erg s]=rsqu(x, [y1 ... yn], 'multilevel')
%  x    is a [NCasesTot x 1] matrix of the values assumed by the
%       independent variable
%  [y1 ... yn] is a [NCasesTot x NDependentVariables] matrix of the values
%       assumed by the dependent variables

% Marco Mattiocco, m.mattiocco@hsantalucia.it
% $Revision: 1.1 $  $Date: 2006/03/17 09:03:50 $
% $Revision: 1.1 $  $Date: 2006/03/17 09:03:50 $
% $Revision: 1.1 $  $Date: 2006/03/17 09:03:50 $

if nargin<2
    return;
end

for sampAndCH=1:size(NDependentVariables, 1)
    [ressq(sampAndCH) sgnR2(sampAndCH)]=rsqu_f(IndependentVariable,NDependentVariables(sampAndCH, :)','multilevel');
end
function [R2, s]= rsquare(X, Y);
% RSQUARE computes explained variance
%  Usage: [R2, s]= rsquare(X, Y);
%  X is a [NCases x NIndependentVariables] matrix of independend values
%  Y is a [NCases x NDependentVariables] matrix of dependend values
%  R2 is a [1 x NDependentVariables] matrix of Rsquare values
%  s is a [NDependentVariables x NIndependentVariables] matrix of signs,
%      indicating the direction of the coupling between IVars and DVars

% Febo Cincotti 2004-12-16
% f.cincotti@hsantalucia.it
ncases=size(X, 1);
X1 = [ones([ncases 1])  X];
beta = X1\Y;% [2 x nDV]; each column is [b0 b1]' | y=b0+b1*x1+z
Yhat = X1*beta;
Z = Yhat - Y;
R2=1-var(Z)./var(Y);
s=sign(beta(2:end, :));
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: UGenericSignal.m 2007-111-26 12:31:37EST schalk $ 
%% 
%% File: UGenericSignal.m 
%% 
%% Author: Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function determines the r^2 values of two
%% distributions
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function erg = rsqu(q, r)
%RSQU   erg=rsqu(r, q) computes the r2-value for
%       two one-dimensional distributions given by
%       the vectors q and r


q=double(q);
r=double(r);

sum1=sum(q);
sum2=sum(r);
n1=length(q);
n2=length(r);
sumsqu1=sum(q.*q);
sumsqu2=sum(r.*r);

G=((sum1+sum2)^2)/(n1+n2);

erg=(sum1^2/n1+sum2^2/n2-G)/(sumsqu1+sumsqu2-G);

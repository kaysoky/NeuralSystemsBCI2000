function [erg, s]= rsqu_f(q, r, varargin)
% RSQU computes explained variance for one discrete explanatory variable
%  Usage: [erg s]=rsqu(Y1, Y2)
%  Y1  is a [NCases1 x NDependentVariables] matrix of values assumed by
%      independent variables (one per column) in correspondence of the first
%      level of the independend variable
%  Y2  is a [NCases2 x NDependentVariables] matrix of values assumed by
%      independent variables (one per column) in correspondence of the
%      second  level of the independend variable
%  erg is a [1 x NDependentVariables] matrix of the explained variance
%  s   is a [1 x NIndependentVariables] matrix the sign of the link between
%      the dependent variable and the independent variables
%
%  Usage: [erg s]=rsqu(Y1, Y2, ..., Yn)
%      with respect to the previous case, more than two levels are allowed
%      for the independent variable
%
%  Usage: [erg s]=rsqu(x, [y1 ... yn], 'multilevel')
%  x    is a [NCasesTot x 1] matrix of the values assumed by the
%       independent variable
%  [y1 ... yn] is a [NCasesTot x NDependentVariables] matrix of the values
%       assumed by the dependent variables
%
%  See also: RSQUARE

% Febo Cincotti 2004-12-16
% f.cincotti@hsantalucia.it
% 
% The first Usage is backward compatible with rsqu() by Gerwin Schalk, on
% whose code is based one part of this function.
if nargin<2 
   Mario_printf('Not enough input arguments','error')
elseif nargin==2
   q=double(q);
   r=double(r);
   
   sum1=sum(q);
   sum2=sum(r);
   n1=length(q);
   n2=length(r);
   sumsqu1=sum(q.^2);
   sumsqu2=sum(r.^2);
   
   G=((sum1+sum2).^2)/(n1+n2);
   
   erg=(sum1.^2/n1+sum2.^2/n2-G)./(sumsqu1+sumsqu2-G);
   s=sign(sum1-sum2);%modificato da mm 11-01-05 prima era s=sign(sum2-sum1)
elseif nargin==3 & ischar(varargin{1}) & strcmpi('multilevel', varargin{1})
   [erg, s]= rsquare(q, r);
   s=-s;             %modificato da mm 11-01-05 prima era s=-s
else% nargin >2
   %====
   % Convert input to cell array
   Y{1}=q;
   Y{2}=r;
   for yndx=3:nargin
      Y{yndx}=varargin{yndx-2};
   end% for
   %
   % 
   nlevels= length(Y);% number of levels of independent variable
   nDV=size(Y{1}, 2);% number of dependent variables
   Y1=zeros([0 nDV]);
   X1=zeros([0 1]);
   for lev=[1:nlevels]
      Y1=[Y1; Y{lev}];
      curncases=size(Y{lev}, 1);
      X1=[X1; lev*ones([curncases 1])];
   end% for
   %
   ncases=length(Y1);% number of cases (samples of each variable)
   [erg, s]= rsquare(X1, Y1);
end% if



% === test: ===
% a1=3; b1=7;
% a2=-4; b2=2;
% x=round(rand([100 1]));
% y1 = a1*x+b1 + .1*a1*randn(size(x));
% y2 = a2*x+b2 + .3*a2*randn(size(x));
% Y=[y1 y2];
% zindex=[x==0];
% [R2, s]= rsqu(Y(zindex, :), Y(~zindex, :))



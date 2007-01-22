function out=clip(in,mn,mx,mnOut,mxOut)
% CLIP returns input matrix clipped to min & max values
%
%    out = clip(in, min, max)
%       returns [min] ([max]) for elements that are less than [min] ([max])
%
%    out = clip(in, min, max, defMinValue, defMaxValue)
%       returns [defMinValue] ([defMaxValue]) for elements that are less than [min] ([max])
%       if only defMinValue is specified, returns it for both cases

if nargin<5
   if nargin==4; 
      mxOut=mnOut;
   else
      mnOut=mn;
      mxOut=mx;
   end
end

%un solo valore e' specificato esplicitamente
if ischar(mnOut) && strcmp(mnOut, 'min')
   mx=min(min(in));
end
if ischar(mxOut) && strcmp(mxOut, 'max')
   mx=max(max(in));
end

%controllo coerenza
if mn>mx
   warning('CLIP.M - min clipping value is greater then max');
end

% % % %copiatura valori in uscita
% % % out=in;
% % % if isempty(mnOut)
% % % out(find(out<mn))=[];
% % % else
% % % out(find(out<mn))=mnOut;
% % % end
% % % if isempty(mxOut)
% % % out(find(out>mx))=[];
% % % else
% % % out(find(out>mx))=mxOut;
% % % end

%copiatura valori in uscita
out=in;
%clipping
out(find(out<mn))=mnOut;
out(find(out>mx))=mxOut;
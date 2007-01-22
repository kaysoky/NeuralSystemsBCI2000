function validPOS=ValidatePos(POS)
%  usage: validPOS=VALIDATEPOS(POS);

if nargin<1
   return;
end

validPOS=POS;

%eventuale creazione del campo elLbls della struttura POS per etichettare
%gli elettrodi di misura
if ~isfield(POS,'elLbls')
   nEl=size(POS.elCoords,1);
   validPOS.elLbls=strtrim(cellstr(num2str((1:nEl)')));
end

%eventuale creazione del campo refCoords della struttura POS per indicare
%le coordinate dei riferimenti craniometrici
if ~isfield(POS,'refCoords')
   Dx=abs(min(POS.elCoords(:,1)));
   Dy=(max(POS.elCoords(:, 2))-min(POS.elCoords(:,2)))/2;
   validPOS.refCoords=diag([Dy Dy Dx])*[0 -1 0; 0 1 0; -1 0 0];
end

%eventuale creazione del campo refLbls della struttura POS per etichettare
%i riferimenti craniometrici
if ~isfield(POS,'refLbls')
   validPOS.refLbls={'A1' 'A2' 'NS'};
end

% % % %eventuale crezione del campo name della struttura POS per descrivere il
% % % %montaggio
% % % if ~isfield(POS,'name')
% % %    if isfield(POS,'tag')
% % %       validPOS.name=POS.tag;
% % %    else
% % %       validPOS.name='Unknown montage';
% % %    end
% % % end

% % % if ~isfield(POS,'valid')
% % %    validPOS.validated=find(~isnan(sum(POS.elCoords,2)));
% % % end

% % % validPOS.elCoords=validPOS.elCoords(validPOS.validated,:);
% % % validPOS.elLbls=validPOS.elLbls(validPOS.validated);
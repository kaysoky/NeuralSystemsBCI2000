function intMat = sphInterpMat(elCoords, intCoords, lambda, modality)
%Call: intMat = sphInterpMat(elCoords, intCoords, lambda, modality)
%Default:
%SPLORD=2;
%LPORD=79;

%deve stare a zero se voglio considerare anche la diagonale principale
DIAGFLAG=0;

if nargin<3
    lambda=0;
end
if nargin<4
    modality='sphlap';
end

tic
msg=sprintf('\nStarting %s', mfilename);
disp(msg);

% Compute spline coefficients
nChans = size(elCoords, 1);
%matrice quadrata di dimensioni nChans,nChans che viene utilizzata per fare
%una maschera; in pratica vengono considerati solo gli elementi sotto la
%diagonale principale, e quest'ultima nel caso DIAGFLAG sia nulla.
triLNdx=find(tril(ones(nChans),-DIAGFLAG));
%indici della diagonale principale
diagNdx=find(tril(eye(nChans))); % indici della diagonale di extG
%calcolo della matrice dei coseni direttori, il generico elemento è dato
%dal prodotto scalare tra i versori rappresentativi dei corrispondneti
%punti. Ad esempio l'elemento i,j è il prodotto scalare tra i versori
%corrispondenti ai vettori che identificano il punto i e quello j. una
%coordinata identifica un vettore, quello che si ottiene unendo l'origine
%con l'estremo libero, ovvero il punto stesso.
dc = dircos(elCoords, elCoords); %Ej*Ei
%considero solo i coseni direttori tra i vettori rappresentativi delle
%posizioni elettrodiche al di sotto della diagonale principale e su di esa;
%le altre info sono ridondanti. questi dati sono ordinati in un vettore a
%partire dal dc tra vett1 e vett1, poi quello tra vett1 e vett2, e così
%via, fino a quello tra vettN e vettN
dcVec=dc(triLNdx);
%mexfunction, è una libreria.
[gVec, hVec] = funG_H(dcVec);
g=zeros(nChans);% inizializza
g(triLNdx)=gVec;% metà inferiore
g=(g+tril(g, -1)');%.../(4*pi);%  rende simmetrica ricopiando in alto
g(diagNdx)=g(diagNdx)+lambda;
extG=[ ...
    ones([nChans 1]), g; ...
    0, ones([1 nChans]) ...
    ];
invExtG=inv(extG);

% Compute interpolation coefficients 
msg=sprintf('\nStarting coefficient computation at %.2f s', toc);
disp(msg);
nInterp = size(intCoords, 1);
intDc = dircos(intCoords, elCoords); %E*Ei
intDcVec=intDc(:);
[intGVec, intHVec] = funG_H(intDcVec); %mexfunction

%Compute interpolation matrix
msg=sprintf('\nStarting interp. matrix computation at %.2f s', toc);
disp(msg);
switch lower(modality)
    case 'sphlap'
        intH=reshape(intHVec,  [nInterp nChans]);
        intMat=[intH] * invExtG(2:end, 1:nChans);
    case 'sphinterp'
        intG=reshape(intGVec,  [nInterp nChans]);
        intMat=[ones([nInterp 1]) intG] * invExtG(:, 1:nChans);
    otherwise
        error('Unknown modality')
end
msg=sprintf('\nExiting %s in %.2f s', mfilename, toc);
disp(msg);
function b=mario_stepwise(memSG,Regressor)
% ------------------------------------------
% X=unpkSigStruct.SIG.memSG(5:15,19:40,:);
% y=unpkSigStruct.Regressor;
% mario_stepwise(X,y);
% ------------------------------------------

D2memSG=reshape(memSG,[size(memSG,1)*size(memSG,2) size(memSG,3)]);
D2targetsVect=Regressor';
b=stepwisefit(D2memSG',D2targetsVect);
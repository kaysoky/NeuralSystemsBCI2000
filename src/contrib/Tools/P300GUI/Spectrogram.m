%%%%%  function to plot a 2D matrix of tiles
%%%%%  Dean Krusienski   1/2005              
%%%%%  Wadsworth Center/NYSDOH   

function Spectrogram(features,cax,ax1,ax2)

r=size(features,1);
c=size(features,2);
val=mean(mean(features));

features(r+1,:)=val*ones(1,c);
features(:,c+1)=val*ones(1,r+1)';


if isempty(ax1)==1
    surf(1:c+1,1:r+1,features)
else
    surf(ax1,ax2,features)
end

view(2)
colormap jet;

if isempty(cax)==1
    caxis([min(min(features)) max(max(features))])
else
    caxis(cax)
end
% colorbar;
opengl neverselect
if isempty(ax1)==1
    axis([1 c+1 1 r+1]);
else
    axis([ax1(1) ax1(length(ax1)) ax2(1) ax2(length(ax2))]);
end




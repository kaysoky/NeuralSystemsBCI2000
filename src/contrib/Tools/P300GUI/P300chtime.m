%%%%%  function to plot the target and standard responses
%%%%%  Dean Krusienski   1/2005
%%%%%  Wadsworth Center/NYSDOH

function P300chtime(AllResponses,Type,windowlen,SamplingRate,tfile,SF,stde)

totch=size(AllResponses,3);
r2=zeros(totch,windowlen(2)-windowlen(1));

AllResponses=single(AllResponses);
Type=single(Type);

fprintf(1,'Calculating r^2...\n')
for uu=1:totch
    r2(uu,:)=corr(Type,AllResponses(:,:,uu)).^2;
end

tpe=find(Type==1);
AvgTargets=mean(AllResponses(tpe,:,:));
SETargets=std(AllResponses(tpe,:,:))/sqrt(length(tpe));
tpe=find(Type==0);
AvgNTargets=mean(AllResponses(tpe,:,:));
SENTargets=std(AllResponses(tpe,:,:))/sqrt(length(tpe));

clear AllResponses Type

AvgTargets=reshape(AvgTargets,windowlen(2)-windowlen(1),totch);
AvgNTargets=reshape(AvgNTargets,windowlen(2)-windowlen(1),totch);
SETargets=reshape(SETargets,windowlen(2)-windowlen(1),totch);
SENTargets=reshape(SENTargets,windowlen(2)-windowlen(1),totch);

range=1000*[windowlen(1):windowlen(2)]/SamplingRate;
range2=1000*[windowlen(1):windowlen(2)-1]/SamplingRate;
r2maxall=max(max(r2));

if SF==3
   car=1;
else 
   car=0;
end

R2fig=figure;
set(gcf,'Name',['r^2 ' tfile ' CAR: ' num2str(car)])
set(gcf,'Position',[40 190 560 420])
Spectrogram(r2,[0 r2maxall],[range],[1:totch+1])
% imagesc(flipdim(r2,1))
% set(gca,'XTickLabel',[2 2.1 2.2 2.3 2.4 2.5])
title({'r^2 Channels vs. Time';'Left click to plot topography and responses';'Right click to end'})
xlabel('time(ms)')
ylabel('channels')
colorbar


button=0;
while button~=3
    figure(R2fig)
    [x,y,button] = ginput(1);
    if button~=3
        x=floor(x*SamplingRate/1000-windowlen(1)+1);
        y=floor(y);
        str = num2str(y);
        figure
        set(gcf,'Name',['Topo ' str ' ' tfile ' CAR: ' num2str(car)])
        set(gcf,'Position',[650 10 330 670])
        
        if ismember(totch,[8 16 32 64])
            subplot(3,1,1)
            topoplotEEG(r2(:,x),['eloc' num2str(totch) '.txt'])
            caxis([0 r2maxall])
            colorbar
            title([num2str(range(x)) ' ms'])
        end

        
        min([AvgNTargets(:,y)-SENTargets(:,y); AvgTargets(:,y)-SETargets(:,y)])
        max([AvgNTargets(:,y)+SENTargets(:,y); AvgTargets(:,y)+SETargets(:,y)])
        
        subplot(3,1,2)        
        if stde==1;          
            errorbar(range2,AvgTargets(:,y),SETargets(:,y),'r')
            set (gca, 'YDir','reverse')
            hold on
            errorbar(range2,AvgNTargets(:,y),SENTargets(:,y),'g')
            axis([min(range2) max(range2)...
                min([AvgNTargets(:,y)-SENTargets(:,y); AvgTargets(:,y)-SETargets(:,y)])...
                max([AvgNTargets(:,y)+SENTargets(:,y); AvgTargets(:,y)+SETargets(:,y)])]);
        else
            plot(range2,AvgTargets(:,y),'r')
            set (gca, 'YDir','reverse')
            hold on
            plot(range2,AvgNTargets(:,y),'g')
        end
        
        set (gca, 'YDir','reverse')
        title(['Channel ' str ' - Targets(red)'])
        xlabel('time(ms)')
        ylabel('Amplitude')

        subplot(3,1,3)
        plot(range2,r2(y,:))
        xlabel('time(ms)')
        ylabel('r^2')
    end
end


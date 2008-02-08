function plotresults(result,MUD,trainfile,testfile)

NumberOfSequences=size(result,1);
gMUD=struct2cell(MUD);
SF=cell2mat(gMUD(6,:));
MA=cell2mat(gMUD(4,:));
DF=cell2mat(gMUD(5,:));
BPF=cell2mat(gMUD(7,:));

sa=unique(SF);
ma=unique(MA);
da=unique(DF);
ba=unique(BPF);


ls=length(sa);
lm=length(ma);
ld=length(da);
lb=length(ba);


sub=0;

if ls>2
    sub=sub+1;
end
% if lm>2|ld>1
%     sub=sub+1;
% end
% if lb>1
%     sub=sub+1;
% end

newsub=1;

if sub>0
    figure
    set(gcf,'Name',['Averages: ' trainfile ' to ' testfile])
    numres=size(result,2);
    clr=['b' 'r' 'g' 'c' 'm' 'y' 'k' 'b' 'r' 'g' 'c' 'm' 'y' 'k'];


    if ls>1
        [a,ind1]=find(SF==0);
        [a,ind2]=find(SF==1);

        % average CAR
        subplot(1,sub,1,'align')
        plot(mean(result(:,ind1),2),'linewidth',2)
        hold on
        plot(mean(result(:,ind2),2),'r','linewidth',2)
        set(gca,'YAxisLocation','right')
        axis([1 NumberOfSequences 0 100])
        title([trainfile ' to ' testfile])
        xlabel('# Sequences')
        ylabel('Percent Correct')
        legend('No CAR','CAR','Location','BestOutside')
        newsub=newsub+1;
    end


%     %% average MA/DF
% 
%     if lm>1|ld>1
%         subplot(1,sub,newsub,'align')
% 
%         if lm>1
%             for kk=1:lm               
%                 [a,ind1]=find(MA==ma(kk));
%                 plot(mean(result(:,ind1),2),clr(kk),'linewidth',2)
%                 hold on
%                 tlab(kk)={['MA: ' num2str(ma(kk))]};
%             end
%         else
%             kk=0;
%         end
% 
% 
%         if ld>1
%             for gg=1:ld
%                 [a,ind1]=find(DF==da(gg));
%                 plot(mean(result(:,ind1),2),clr(kk+gg),'linewidth',2)
%                 hold on
%                 tlab(kk+gg)={[' DF: ' num2str(da(gg))]};
%             end
% 
%         end
%         set(gca,'YAxisLocation','right')
%         axis([1 NumberOfSequences 0 100])
%         title([trainfile ' to ' testfile])
%         xlabel('# Sequences')
%         ylabel('Percent Correct')
%         legend(tlab,'Location','BestOutside')
%         newsub=newsub+1;
%     end
% 
% 
%     %%BPF
%     if lb>1
%         if lb>1
%             for kk=1:lb
%                 [a,ind1]=find(BPF==ba(kk));
%                 plot(mean(result(:,ind1),2),clr(kk),'linewidth',2)
%                 hold on
%                 xlab(kk)={['RS: ' num2str(ba(kk))]};
%             end
% 
%         end
%         set(gca,'YAxisLocation','right')
%         axis([1 NumberOfSequences 0 100])
%         title([trainfile ' to ' testfile])
%         xlabel('# Sequences')
%         ylabel('Percent Correct')
%         legend(xlab,'Location','BestOutside')
% 
%     end
end


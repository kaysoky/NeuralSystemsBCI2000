function hfig = SpectraViewer(BCI2k,x,y)

hfig = figure;
TargetLbls = unique(BCI2k.FeatExtraction.States.Regressor);
for i = 1:length(TargetLbls)
    TargetClass{i} = BCI2k.FeatExtraction.Data(:,:,find(BCI2k.FeatExtraction.States.Regressor == TargetLbls(i)));
    Spectrum(:,:,i) = mean(TargetClass{i},3);
    Legenda{i} = ['target' num2str(TargetLbls(i))];
    hold on;
end
semilogy(squeeze(Spectrum(:,y,:)),'.-');
legend(Legenda);
grid on;
set(hfig,'NumberTitle','off');
set(hfig,'Name',sprintf('Channel %d Spectra',y));
xlabel('Frequency Bin');
ylabel('Amplitude');
set(gca,'XTick',[1:size(Spectrum,1)]);
set(gca,'XTickLabel',[1:size(Spectrum,1)]);
xticklabel_rotate;
set(gcf,'renderer','zbuffer');
hold off;
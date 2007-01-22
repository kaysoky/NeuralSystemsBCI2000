hfig = figure;
TargetLbls = unique(FeatExtraction.States.Regressor);
for i = 1:length(TargetLbls)
    TargetClass{i} = FeatExtraction.Data(:,:,find(FeatExtraction.States.Regressor == TargetLbls(i)));
    TargetMean{i} = mean(TargetClass{i},3);
    Spectrum(:,:,i) = TargetMean{i};
    legenda{i} = ['target' num2str(TargetLbls(i))];
    hold on;
end
semilogy(Spectrum(:,x,:));
legend(legenda);
hold off;
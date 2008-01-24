function writeXLS(result,trainfile,testfile,traindir,tlabel,numMUD)
% (C) 2000-2008, BCI2000 Project
% http://www.bci2000.org

warning off MATLAB:xlswrite:AddSheet

name=[traindir trainfile '.xls'];

xlswrite(name, {'Flashes'},testfile,'B1')
xlswrite(name, 1:size(result,1),testfile,'C1')
xlswrite(name, {testfile},testfile,'A2')
xlswrite(name, tlabel',testfile,'B2')
xlswrite(name, result',testfile,'C2')


% example of P3 analysis
%
% get help for P3 function by typing 'help p3'
%
% In this example, the subject focused on the character 'P' (third row, fourth column of the matrix)
% The analysis shows:
%
% 1) A difference between standard and oddball responses (i.e., responses to rows/columns that do or do not contain 'P')
%    See the two averaged waveforms (at Channel 11/Cz) and their statistical difference (in r2) in Figure 1
%
% 2) The topographical distribution of the P300 potential (measured in r2 between standard and oddball) at multiple times (electrode 17/CP1 bad); Figure 2
%
% 3) The discriminability between standard and oddball as a function of time and channels - Figure 3
%
% 4) The responses to the different columns at Cz (stimuli 1-6) and rows (7-12)
%    As expected, the responses are largest for the row/column containing the 'P' 
%    (column 4 (i.e., stimulus 4) and row 3 (i.e., stimulus 9); Figure 4
%
% 5) Averaged responses for each character in the matrix for channel Cz (as the average between respective row and column response)
%
% (C) Gerwin Schalk 10/02-12/04

[res1ch, res2ch, ressqch, stimdata] = p3('aa1.mat', 240, [-1 -1], [11 15], 650, 1, 1, [270 310 350 390 430 470], [3 2], [], 'eloc64.txt', '');

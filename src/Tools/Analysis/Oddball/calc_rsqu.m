function [ressq, amp1, amp2] = calc_rsqu(data1, data2)
%RSQU   [ressq, avgamp1, avgamp2]  = calc_rsqu(data1, data2) calculates the r2-value for
%       two three-dimensional variables (dim1 by dim2 by trial) data1 and data2
%       the result is ressq (dim1, dim2); each element represents the r2 
%       for this particular combination of dim1, dim2 across trials
%       in addition to r2 values, this function also calculates average amplitudes 
%       for each sample and channel, for both data variables (i.e., conditions), and
%       returns these in amp1 and amp2

for ch=1:size(data1, 2)
 for samp=1:size(data1, 1)
    ressq(samp, ch)=rsqu(data1(samp, ch, :), data2(samp, ch, :));
    amp1(samp, ch)=mean(data1(samp, ch, :));
    amp2(samp, ch)=mean(data2(samp, ch, :));
 end
end

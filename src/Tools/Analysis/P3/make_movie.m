% erg = make_movie(moviefilename,  ressq, triallength, samplefreq, FPS, timestretch, displaymin, displaymax, titletxt)
%
% this function creates a movie from a given matrix (samples x channels)
function erg = make_movie(moviefilename, ressq, triallength, samplefreq, FPS, timestretch, displaymin, displaymax, titletxt, eloc_file)

% Generate the movie.
figNumber=1;
%triallength=size(avgdata1, 1);
lengthsamp= triallength;												% length of data in samples
lengthsec = triallength/samplefreq;										% length of data in sec
nframes = floor(lengthsec*FPS*timestretch);								% number of frames in movie 
segmentlength=nframes/FPS/timestretch*samplefreq/nframes;               % length of one segment (i.e., frame) in samples
figure(figNumber);
fig=figure;
set(fig,'DoubleBuffer','on');
set(gca,'xlim', [-80 80], 'ylim', [-80 80], 'NextPlot', 'replace', 'Visible','off');
mov=avifile(moviefilename, 'COMPRESSION', 'Cinepak', 'QUALITY', 100);
%mov.Quality = 100;
for k = 1:nframes-1
 segmentidx=[round((k-1)*segmentlength+1):round(k*segmentlength+1)];		% this frame will capture these samples
 %segment=res4(segmentidx, :)-res1(segmentidx, :);								% calculate difference between up and down target
 segment=ressq(segmentidx, :);
 segmentmean=mean(segment, 1);														% calculate the mean for this segment for each channel
 idx=find(segmentmean >= displaymax);												% the signal can't be larger or smaller than the max
 segmentmean(idx)=displaymax;
 idx=find(segmentmean <= displaymin);
 segmentmean(idx)=displaymin;
 topoplot(segmentmean, eloc_file, 'maplimits', [displaymin, displaymax], 'style', 'straight');
 titletxt2=sprintf('Frame %d/%d', k, nframes);
 title(titletxt2);
 cur_frame=getframe(gcf);										% get the current frame (i.e., screen shot)
 mov=addframe(mov, cur_frame);
end;

mov = close(mov);
erg = 1;

function erg = make_movie_ext(moviefilename,  ressq, avgamp1, avgamp2, channel, triallength, samplefreq, FPS, timestretch, displaymin, displaymax, titletxt)
% erg = make_movie_ext(moviefilename,  ressq, avgamp1, avgamp2, channel, triallength, samplefreq, FPS, timestretch, displaymin, displaymax, titletxt)
%
% this function creates a movie from a given matrix (samples x channels)
%
% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

% Generate the movie.
figNumber=1;
%triallength=size(avgdata1, 1);
lengthsamp= triallength;												% length of data in samples
lengthsec = triallength/samplefreq;										% length of data in sec
nframes = floor(lengthsec*FPS*timestretch);								% number of frames in movie 
segmentlength=nframes/FPS/timestretch*samplefreq/nframes;               % length of one segment (i.e., frame) in samples
figure(figNumber);
%fig=figure;
%set(gcf,'Position', [100 100 500 550]);
set(gcf,'Position', [100 100 550 500]);
set(gcf,'DoubleBuffer', 'on');
set(gca,'xlim', [-80 80], 'ylim', [-80 80], 'NextPlot', 'replace', 'Visible','off');
mov=avifile(moviefilename, 'COMPRESSION', 'Cinepak', 'QUALITY', 100);
%mov.Quality = 100;
cursorposx=0;
cursorposy=50;
cursorvelx=100/nframes;
cursorsize=5;
timems=[1:nframes]./samplefreq.*1000+150;
for k = 1:nframes-1
 cur_time=k/samplefreq*1000+150;
 segmentidx=[round((k-1)*segmentlength+1):round(k*segmentlength+1)];		% this frame will capture these samples
 %segment=res4(segmentidx, :)-res1(segmentidx, :);								% calculate difference between up and down target
 segment=ressq(segmentidx, :);
 segmentmean=mean(segment, 1);														% calculate the mean for this segment for each channel
 idx=find(segmentmean >= displaymax);												% the signal can't be larger or smaller than the max
 segmentmean(idx)=displaymax;
 idx=find(segmentmean <= displaymin);
 segmentmean(idx)=displaymin;
 % plot the cursor
 %fig=subplot('position', [0.07 0.1 0.84 .2]);
 %cla(fig);
 %plot(timems, avgamp1(:, channel), 'r.');
 %hold on;
 %plot(timems, avgamp2(:, channel), 'g.'); 
 %myline=rectangle('Curvature', [0 0], 'Position', [cur_time 1 1 max(avgamp1(:, channel))]);
 %set(myline, 'LineWidth', 3);
 %set(myline, 'FaceColor', 'blue');
 %set(myline, 'EdgeColor', 'blue');
 %axis([min(timems) max(timems) min(avgamp2(:, channel)) max(avgamp1(:, channel))]);
 %set(fig, 'YTickLabel', []);
 %xlabel('Time [ms]');
 %ylabel('Amplitude at C3');
 if (cur_time > 0)
    titletxt=sprintf('Target appears on the screen (%d ms)', round(cur_time));
 end
 if (cur_time > 1050)
    titletxt=sprintf('Cursor starts to move (%d ms)', round(cur_time));
 end
 if (cur_time > 3000)
    titletxt=sprintf('Reward period (%d ms)', round(cur_time));
 end
 texthandle=title(titletxt);
 set(texthandle, 'FontSize', 18);
 set(texthandle, 'FontWeight', 'bold');
 %texthandle=text(10,10,titletxt);
 %rectangle('Curvature', [1 1], 'Position', [cursorposx-cursorsize/2 cursorposy-cursorsize/2 cursorsize cursorsize])
 %axis([0 100 0 100]);

 % plot the topo
 %fig=subplot('position',[0 .3 1 .7]);
 axis off;
 topoplot(segmentmean, 'eloc64.txt', 'maplimits', [displaymin, displaymax], 'style', 'straight');
 titletxt2=sprintf('%s Frame %d/%d', titletxt, k, nframes);
 %title(titletxt);
 cur_frame=getframe(gcf);										% get the current frame (i.e., screen shot)
 mov=addframe(mov, cur_frame);
 cursorposx=cursorposx+cursorvelx;
end;

mov = close(mov);
erg = 1;

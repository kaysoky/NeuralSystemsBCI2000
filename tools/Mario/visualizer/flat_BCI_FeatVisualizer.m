function VisParams = flat_BCI_FeatVisualizer(BCI2k,LeftEvent,MiddleEvent,RightEvent)

if isfield(BCI2k,'Visualizer')
    VisParams = BCI2k.Visualizer;
end

StatResults = BCI2k.Statistics.Data;
bdfHandle = @ClickedId;

SurfH=surf([1:size(StatResults, 1)],[1:size(StatResults, 2)],double(StatResults)');
axis([1 size(StatResults, 1) 1 size(StatResults, 2)]);
axis ij;
set(SurfH, 'ButtonDownFcn', {bdfHandle, 'ButtonDown',BCI2k,LeftEvent,MiddleEvent,RightEvent});

if isfield (BCI2k,'Visualizer') && isfield(BCI2k.Visualizer,'FeatVisualizer')
    if ~isfield(BCI2k.Visualizer.FeatVisualizer,'Shading')
        shading faceted;
        VisParams.FeatVisualizer.Shading = 'faceted';
    else
        eval(['shading ',BCI2k.Visualizer.FeatVisualizer.Shading]);
    end

    if ~isfield(BCI2k.Visualizer.FeatVisualizer,'ColorScaling')
        MAXIMUM=max(max(abs(StatResults)));
        caxis([-1 1]*MAXIMUM);
        VisParams.FeatVisualizer.ColorScaling.Min = -1 * MAXIMUM;
        VisParams.FeatVisualizer.ColorScaling.Max = +1 * MAXIMUM;
    else
        caxis([BCI2k.Visualizer.FeatVisualizer.ColorScaling.Min BCI2k.Visualizer.FeatVisualizer.ColorScaling.Max]);
    end

    if isfield(BCI2k.Visualizer.FeatVisualizer,'Colormap')
        eval(['colormap ',BCI2k.Visualizer.FeatVisualizer.Colormap]);
    else
        colormap Default;
        VisParams.FeatVisualizer.Colormap = 'Default';
    end
else
    shading faceted;
    MAXIMUM=max(max(abs(StatResults)));
    caxis([-1 1]*MAXIMUM);
    colormap Default;
    VisParams.FeatVisualizer.Shading = 'Faceted';
    VisParams.FeatVisualizer.ColorScaling.Min = -1 * MAXIMUM;
    VisParams.FeatVisualizer.ColorScaling.Max = +1 * MAXIMUM;
    VisParams.FeatVisualizer.Colormap = 'Default';
end
colorbar;
xlabel('Frequency Bin');
ylabel('Channel Num');
% set(gca,'XTick',[1:size(StatResults,1)]);
% set(gca,'XTickLabel',[1:size(StatResults,1)]);
% xticklabel_rotate;
set(gcf,'renderer','zbuffer');

function [x,y,MouseButton] = ClickedId(hObject, eventdata, command, varargin)

BCI2k = varargin{1};
LeftEvent = varargin{2};
MiddleEvent = varargin{3};
RightEvent = varargin{4};

switch(command)
    case 'ButtonDown'
        MatAxHndl=get(hObject, 'Parent');
        MatFigHndl=get(MatAxHndl, 'Parent');
        MouseButton=get(MatFigHndl,'SelectionType');
        UD=get(MatFigHndl, 'UserData');
        CurPoint=get(MatAxHndl, 'CurrentPoint');
        Coords=CurPoint(1, [1 2]);
        Coords=Coords-0.5;
        RoundCoord=round(Coords);
        x = RoundCoord(1);
        y = RoundCoord(2);
        if strcmpi(MouseButton,'normal') & ~isempty(LeftEvent)
            ChildFigH = eval([LeftEvent,'(','BCI2k',',','x',',','','y',')']);
        end
        if strcmpi(MouseButton,'extend') & ~isempty(MiddleEvent)
            ChildFigH = eval([MiddleEvent,'(','BCI2k',',','x',',','','y',')']);
        end
        if strcmpi(MouseButton,'alt') & ~isempty(RightEvent)
            ChildFigH = eval([RightEvent,'(','BCI2k',',','x',',','','y',')']);
        end
end

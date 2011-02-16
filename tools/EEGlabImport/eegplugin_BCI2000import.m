function vers = eegplugin_BCI2000import(fig, trystrs, catchstrs)
% $BEGIN_BCI2000_LICENSE$
% 
% This file is part of BCI2000, a platform for real-time bio-signal research.
% [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
% 
% BCI2000 is free software: you can redistribute it and/or modify it under the
% terms of the GNU General Public License as published by the Free Software
% Foundation, either version 3 of the License, or (at your option) any later
% version.
% 
% BCI2000 is distributed in the hope that it will be useful, but
%                         WITHOUT ANY WARRANTY
% - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
% A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License along with
% this program.  If not, see <http://www.gnu.org/licenses/>.
% 
% $END_BCI2000_LICENSE$

    vers = 'BCI2000import1.00';
    if nargin < 3
        error('eegplugin_BCI2000import requires 3 arguments');
    end;
    
    % add folder to path
    % ------------------
    if ~exist('BCI2000import')
        p = which('eegplugin_BCI2000import.m');
        p = p(1:findstr(p,'eegplugin_BCI2000import.m')-1);
        addpath([ p 'BCI2000import1.00' ] );
    end;
    
    % find import data menu
    % ---------------------
    menu = findobj(fig, 'tag', 'import data');
    
    % menu callbacks
    % --------------
    comcnt = [ trystrs.no_check '[EEG] = BCI2000import;'     catchstrs.new_and_hist ];
    
    % create menus
    % ------------
    uimenu( menu, 'label', 'From BCI2000 .DAT file', 'callback', comcnt, 'separator', 'on' );

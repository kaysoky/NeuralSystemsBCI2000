function = save_bcidat( varargin )
%SAVE_BCIDAT Save Matlab workspace variables into a BCI2000 data file. 
%
%  save_bcidat( 'filename', signal, states, parameters )
%
%  Saves signal, state, and parameter data into the named file.
%  Signal data is always interpreted as raw data, i.e. it will be written
%  into the output file unchanged.
%
%  File format is deduced from the output file's extension, which may be 
%  .dat, .edf, or .gdf. When no extension is recognized, the 
%  BCI2000 dat file format is used.
%
%
%  The save_bcidat function is part of the BCI2000 project. 
%  (C) 2000-2008, BCI2000 Project
%  http://www.bci2000.org

%  This is a help file documenting the functionality contained in
%  save_bcimat.mex.
%  $Id$
%
error( 'There is no save_bcidat mex file for your platform available.' );

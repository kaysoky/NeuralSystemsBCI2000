function readudp
% Demo showing how to access AppConnector and P3Speller information through
% a UDP socket from Matlab.
%
% This demo is part of the BCI2000 project.
% (C) 2000-2009, BCI2000 Project
% http://www.bci2000.org
% $Id$

ip = 'localhost';
port = 20320;
% Create and open a UDP object that connects to BCI2000.
u = udp( ip, 20319, 'LocalPort', port, 'Terminator', 'CR/LF', 'Timeout', 10 );
fopen( u ); 
% Read data until timeout occurs.
s = fgetl( u );
while( s~=-1 ) 
  fprintf( '%s', s ); 
  s = fgetl( u );
end 
% Close and delete the UDP object.
fclose( u ); 
delete( u ); 

function readudp

u = udp('localhost',20319,'LocalPort',20320,'Terminator','CR/LF','Timeout',10);
fopen(u); 
s = fgetl(u);
while (s~=-1) 
  fprintf('%s', s); 
  s = fgetl(u);
end 
fclose(u); 
delete(u); 

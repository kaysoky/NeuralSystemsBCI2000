The DAS source module handles A/D boards from Measurement Computing (previously called ComputerBoards).

The DAS source module uses Measurement Computing's system wide driver and configuration files. Previous versions would only work with a certain version of the cbw32.dll file; now (Sep 26, 2003) it should work with any recent and future versions. If there is an error message saying that loading the DLL failed, you need to download and install a recent version of InstaCal from http://www.measurementcomputing.com (free of charge), and then use it to configure your board.
IMPORTANT:
For the source module to work properly, you need to _delete_ any files cbw32.dll and cb.cfg from the directory containing the source module (these files may be left over from previous versions). It would also be a good idea to delete all other files called cbw32.dll and cb.cfg, except the ones located in the directory where Measurement Computing's InstaCal.exe resides (usually C:\mmc or C:\Program Files\mmc).

Boards with large data buffers ('FIFOs') such as the PC-Card-DAS16/16 are supported. Previous versions could not handle a situation where the board's FIFO would hold more than twice the amount of data corresponding to a sample block (as determined by the settings in the parameters SampleBlockSize and SoftwareCh); instead of immediately sending each block to the signal processing module, it would wait for multiple blocks to arrive before sending all of them within a short period of time. This has been fixed now.

Support for 12-bit boards (untested). The previous version would handle data from 12-bit boards as if it were 16-bit data, i.e. it would output the data multiplied by 16, and treat the least 4 bits as data instead of the channel number they actually contain. If you corrected for this using the SourceChGain and SourceChOffset parameters you should adapt these settings to the new behavior. 
Please, let me know about any experiences with 12-bit boards.

Boards it works with:
The file EEGsource/DASSource/Boards.txt contains a list of boards that have been reported to work with the DASSource module. If you use a board not listed there it would be very nice if you reported your experiences to juergen.mellinger@uni-tuebingen.de.


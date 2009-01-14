5DT Data Glove 5 Ultra
======================

Author: Peter Brunner
        pbrunner@wadsworth.org
  Date: 05/17/2007 (Initial Release)
        01/14/2009 (Added Support for the 14 sensor data glove) 
 Files: ./DataGlove5DTUFilter.cpp
        ./DataGlove5DTUFilter.h
        ./DataGlove5DTUSignalProcessing.cpp
        ./UFilterHandling.cpp
        ./UFilterHandling.h
        ./fglove.h
        ./fglove.dll
        ./5DT Data Glove Ultra - Manual.pdf
        ./readme.txt

Description:
------------
The 5DT Data Glove 5 Ultra is a 5 sensor data glove with 1 sensor per finger that measures average of knuckle and first joint movement. The data glove is interfaced over its full-speed USB 1.1 interface to the computer. The sensor resolution is 12 bit with at least 8 bit dynamic range. The temporal resolution is 13.33ms which equals a sampling rate of 75 Hz. 

The 5DT Data Glove 14 Ultra is a 14 sensor data glove with 2 sensors per finger  that measures average of knuckle and first and second joint movement and the abduction between the 5 fingers. The data glove is interfaced over its full-speed USB 1.1 interface to the computer. The sensor resolution is 12 bit with at least 8 bit dynamic range. The temporal resolution is 13.33ms which equals a sampling rate of 75 Hz. 


Vendor:
-------
The 5DT Data Glove can be purchased from 5DT (www.5dt.com) as a right handed (5DT Data Glove Ultra 5 Right-handed) and left handed (5DT Data Glove Ultra 5 Left-handed) version for 995 USD each (as of 5/17/2007). The warranty involves a 30 day customer satisfaction guarantee and 1 year product warranty. It comes in a hard-plastic transport case (28cm x 10cm x 10cm) with the data glove, a manual, a CD and a 3m USB cable. 

Integration into BCI2000:
-------------------------
The 5DT Data Glove 5 Ultra is integrated as a filter (DataGlove5DTU) into BCI2000. An BCI2000 module (e.g. application) utilizing this filter has to register the filter (i.e. RegisterFilter( DataGlove5DTUFilter, 4 ); ) and add the DataGlove5DTUFilter.cpp file to the project. The dynamic link library fglove.dll has to be in the path of the executable module. 

Parameterization:
-----------------
The 5DT Data Glove 5 Ultra is configured in the panel HumanInterfaceDevices in the section DataGlove5DTUFilter. 

DataGloveEnable   = checked/unchecked
DataGloveHandType = left/right

State Variables:
----------------
For each of the 5 (14) sensors of the 5DT Data Glove 5 (14) Ultra, one state variable is stored with the data.

---------------------------------------------------------------
StateName     == Finger       | 5 sensor | 14 sensor 
---------------------------------------------------------------
GloveSensor1  == thumb        | flexure  | flexure lower joint   
GloveSensor2  == index        | flexure  | flexure at knuckle 
GloveSensor3  == middle       | flexure  | flexure at knuckle
GloveSensor4  == ring         | flexure  | flexure at knuckle
GloveSensor5  == little       | flexure  | flexure at knuckle
GloveSensor6  == thumb        | N/A      | flexure second joint
GloveSensor7  == index        | N/A      | flexure second joint
GloveSensor8  == middle       | N/A      | flexure second joint
GloveSensor9  == ring         | N/A      | flexure second joint
GloveSensor10 == little       | N/A      | flexure second joint 
GloveSensor11 == thumb        | N/A      | abduction
GloveSensor12 == index/middle | N/A      | abduction 
GloveSensor13 == middle/ring  | N/A      | abduction
GloveSensor14 == ring/ring    | N/A      | abduction
---------------------------------------------------------------

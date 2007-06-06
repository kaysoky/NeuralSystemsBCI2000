Most EEG amplifiers have a line noise hum filter, or "notch filter", built in.
For the rare cases in which this is not the case, BCI2000 offers a software notch filter.

Unlike other filters, the notch filter will be applied immediately after data acquisition, 
and it will modify the data which will be saved to disk, i.e., it will behave just as if 
it were a hardware filter built into the amplifier.

The SourceFilter class is defined in BCI2000/SignalProcessing/misc/SourceFilter.cpp.
To make a notch filter available in a particular source module, add this file, and the 
two associated files IIRFilter.cpp and FilterDesign.cpp, to the source module's list of
project files.
To activate the filter, set the "NotchFilter" parameter to the value appropriate for your 
country (1 for 50Hz, or 2 for 60Hz).
Additionally to the notch filter, there is a HP filter available which may be activated
from the "HighPassFilter" parameter.
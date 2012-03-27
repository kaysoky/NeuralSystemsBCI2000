BCI2000_ADD_CMDLINE_FILTER( CustomIIRFilter FROM ..
                            INCLUDING MATH
                            EXTRA_SOURCES ../CustomIIR.cpp
                            EXTRA_HEADERS ../CustomIIR.h
                           )
BCI2000_ADD_CMDLINE_FILTER( CustomFIRFilter FROM .. )
BCI2000_ADD_CMDLINE_FILTER( HilbertFilter FROM .. )
BCI2000_ADD_CMDLINE_FILTER( DiffFilter FROM .. )

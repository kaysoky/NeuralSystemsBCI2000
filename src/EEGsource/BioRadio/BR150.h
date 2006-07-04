////////////////////////////////////////////////////////////////////////////////
// $Id$
// $Log$
// Revision 1.1  2006/07/04 18:44:25  mellinger
// Put files into CVS.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BR150_H
#define BR150_H

#include "BR_defines.h"
#include <string>
#include <windows.h>

class BR150
{
  public:
                 //Constructor
                 BR150();
                 //Destructor
                 ~BR150();
                 // starts the bioRadio communication
         int     start(char*);
                 // kills communications, destroys radio object
         int     stop(void);
                 // purges internal buffer
         void    purge(void);
                 // Returns the number of samples read
         int     samplesRead(void);
                 // Get the running state of bioradio
         bool    getState(void);
                 // gets pointer of filled buffer
         double* getData(int block, int chans);
                 // gets pointer of filled buffer
         double* getData(void);
                 // programs bioradio using a configuration file
         int     program(const std::string& config);
  private:       // buffer merging (private: isnt required outside the object)
         void    bufferMerge(double* buffer1,double* buffer2, int start, int finish);

  private:
                // Type DWORD, is equated to the radio object
         static DWORD br150;
                // Storage of the success flags
         bool   flags[3];
                // Pointer to a char buffer that stores the COM port, eg "COM3"
         char   *port;
                 // An array that holds the raw data obtained from the bioRadio
         double  data[BUFFER_SIZE];
                 // Number of samples collected
         int      numRead;
                 // Stores the running state of the bioradio
         static   bool runningState;
};

#endif // BR150_H


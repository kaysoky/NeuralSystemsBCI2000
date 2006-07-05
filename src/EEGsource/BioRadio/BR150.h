////////////////////////////////////////////////////////////////////////////////
// $Id$
// $Log$
// Revision 1.2  2006/07/05 15:21:19  mellinger
// Formatting and naming changes.
//
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
        int     Start(const char*);
                // kills communications, destroys radio object
        int     Stop(void);
                // purges internal buffer
        void    Purge(void);
                // Returns the number of samples read
        int     SamplesRead(void) const;
                // Get the running state of bioradio
  static bool   GetState(void);
                // gets pointer of filled buffer
  const double* GetData(int block, int chans);
                // gets pointer of filled buffer
  const double* GetData(void);
                // programs bioradio using a configuration file
        int     Program(const std::string& config);
                // determines the port that the bioradio is attached to
  const char*   PortTest(int port);

 private:       // buffer merging (private: isnt required outside the object)
  static void   BufferMerge(double* buffer1,const double* buffer2, int start, int finish);

 private:
                // Storage of the success flags
        bool    mFlags[3];
                // Pointer to a char buffer that stores the COM port, eg "COM3"
  const char*   mpPort;
                // An array that holds the raw data obtained from the bioRadio
        double  mData[BUFFER_SIZE];
                // Number of samples collected
        int     mNumRead;
                // Type DWORD, is equated to the radio object
  static DWORD  sBR150;
                // Stores the running state of the bioradio
  static bool   sRunningState;
};

#endif // BR150_H


////////////////////////////////////////////////////////////////////
// File:        bci2000_types.h
// Date:        Jul 21, 2003
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Provides data types and declarations that
//              should go into a file in the "shared" directory.
////////////////////////////////////////////////////////////////////

typedef enum
{
  none = 0,
  status = 1,
  parameter = 2,
  state = 3,
  data = 4,
  visualization = data,
  state_vector = 5,
  system_command = 6,

  signal = 1,
  graph = signal,
  memo = 2,

  int_data = 0,
  float_data = 1

} Descriptor;

typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;

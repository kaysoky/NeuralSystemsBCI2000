/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    UCoreMessage.cpp                                                *
 * Comment:   This unit provides support for the core communication           *
 * Version:   0.16                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.08 - 03/30/2000 - First commented version                               *
 * V0.11 - 06/13/2000 - Support for visualization data                        *
 * V0.15 - 03/29/2001 - Added support for SYSCOMMAND                          *
 * V0.16 - 04/11/2002 - Added a little pause to send and receive functions    *
 *                      (so that processor load doesn't go to 100%)           *
 ******************************************************************************/

//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop

#include "UCoreMessage.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// **************************************************************************
// Function:   COREMESSAGE
// Purpose:    the constructor for the COREMESSAGE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
COREMESSAGE::COREMESSAGE()
: length( 0 ),
  descriptor( 0 ),
  supp_descriptor( 0 )
{
}

// **************************************************************************
// Function:   SetDescriptor
// Purpose:    sets the descriptor for this coremessage
// Parameters: newdescriptor - the new descriptor
// Returns:    N/A
// **************************************************************************
void COREMESSAGE::SetDescriptor(BYTE newdescriptor)
{
 descriptor=newdescriptor;
}


// **************************************************************************
// Function:   SetSuppDescriptor
// Purpose:    sets the supplemental descriptor for this coremessage
// Parameters: newsuppdescriptor - the new supplemental descriptor
// Returns:    N/A
// **************************************************************************
void COREMESSAGE::SetSuppDescriptor(BYTE newsuppdescriptor)
{
 supp_descriptor=newsuppdescriptor;
}


// **************************************************************************
// Function:   GetDescriptor
// Purpose:    returns the descriptor for this coremessage
// Parameters: N/A
// Returns:    descriptor
// **************************************************************************
BYTE COREMESSAGE::GetDescriptor() const
{
 return(descriptor);
}


// **************************************************************************
// Function:   GetSuppDescriptor
// Purpose:    returns the supplemental descriptor for this coremessage
// Parameters: N/A
// Returns:    supplemental descriptor
// **************************************************************************
BYTE COREMESSAGE::GetSuppDescriptor() const
{
 return(supp_descriptor);
}


// **************************************************************************
// Function:   GetLength
// Purpose:    Returns the length of this coremessage
//             This length specifies ONLY the length of the actual data,
//             i.e., it EXcludes the length of the header when it is being
//             transmitted. The length of a coremessage cannot exceed 65kB
// Parameters: N/A
// Returns:    length of the core message
// **************************************************************************
unsigned short COREMESSAGE::GetLength() const
{
 return(length);
}


// **************************************************************************
// Function:   SetLength
// Purpose:    Sets the length of this coremessage
//             This length specifies ONLY the length of the actual data,
//             i.e., it EXcludes the length of the header when it is being
//             transmitted. The length of a coremessage cannot exceed 65kB
// Parameters: newlength - length of the core message
// Returns:    N/A
// **************************************************************************
void COREMESSAGE::SetLength(unsigned short newlength)
{
 length=newlength;
}


// **************************************************************************
// Function:   GetBufPtr
// Purpose:    Gets a pointer to the GetLength() bytes data in the coremessage
// Parameters: N/A
// Returns:    pointer to the data
// **************************************************************************
char *COREMESSAGE::GetBufPtr()
{
 return(buffer);
}


// **************************************************************************
// Function:   SendBufBytes
// Purpose:    this procedure sends EXACTLY length bytes to the specified
//             (non-blocking) socket connection, i.e., it does not return
//             before
// Parameters: Socket - the non-blocking socket
//             buf - the buffer that has been allocated to hold the data
//             length - the number of byes to read from the socket connection
// Returns:    <= 0 ... on error
//             >0   ... on success
// **************************************************************************
int COREMESSAGE::SendBufBytes(TCustomWinSocket *Socket, char *buf, int length)
{
int     bytessent, count=0;

 if (length == 0) return(0);
 while (count < length)
  {
  try
   {
   if (!(Socket->Connected)) return(0);
   bytessent=Socket->SendBuf(buf, length-count);
   }
  catch( TooGeneralCatch& )
   {
   return(0);
   }
  if (bytessent != -1)
     {
     count += bytessent;
     buf += bytessent;
     }
  // if we haven't sent everything on the first try, just pause for 1ms
  // we should give the system some time to run some errands
  if (bytessent < length)
     Sleep(1);
  }

 return(length);
}


// **************************************************************************
// Function:   ReceiveBufBytes
// Purpose:    this procedure reads EXACTLY length bytes from the specified
//             (non-blocking) socket connection, i.e., it does not return
//             before
// Parameters: Socket - the non-blocking socket
//             buf - the buffer that has been allocated to hold the data
//             length - the number of byes to read from the socket connection
// Returns:    length ... on success
//             0 ... on error
// **************************************************************************
int COREMESSAGE::ReceiveBufBytes(TCustomWinSocket *Socket, char *buf, int length)
{
int     bytesread, count=0;

 if (length == 0) return(0);
 while (count < length)
  {
  try
   {
   if (!(Socket->Connected)) return(0);
   bytesread=Socket->ReceiveBuf(buf, length-count);
   }
  catch( TooGeneralCatch& )
   {
   return(0);
   }
  if (bytesread != -1)
     {
     count += bytesread;
     buf += bytesread;
     }
  // if we haven't read everything on the first try, just pause for 1ms
  // we should give the system some time to run some errands
  if (bytesread < length)
     Sleep(1);
  }

 return(length);
}


// **************************************************************************
// Function:   ReceiveBufBytes
// Purpose:    this procedure reads EXACTLY length bytes from the specified
//             Socket stream that has been created for a (blocking) socket
//             connection, i.e., it does not return before
// Parameters: stream - the stream to the blocking socket
//             buf - the buffer that has been allocated to hold the data
//             length - the number of byes to read from the socket connection
// Returns:    length ... on success
//             0 ... on error
// **************************************************************************
int COREMESSAGE::ReceiveBufBytes(TWinSocketStream *stream, char *buf, int length)
{
int     bytesread, count=0;

 if (length == 0) return(0);
 while (count < length)
  {
  try
   {
   bytesread=stream->Read(buf, length-count);
   }
  catch( ESocketError& )
   {
   return(0);
   }
  if (bytesread != 0)
     {
     count += bytesread;
     buf += bytesread;
     }
  else
     return(0);
  // if we haven't read everything on the first try, just pause for 1ms
  // we should give the system some time to run some errands
  if (bytesread < length)
     Sleep(1);
  }

 return(length);
}


// **************************************************************************
// Function:   ReceiveCoreMessage
// Purpose:    given a handle to an open non-blocking Socket connection,
//             receives the content of a coremessage and sets the values
//             of the descriptor, the supplemental descriptor and the length
//             accordingly
//             use ParseMessage() to further process the coremessage
// Parameters: Socket - the non-blocking socket
// Returns:    always ERRCORE_NOERR
// **************************************************************************
int COREMESSAGE::ReceiveCoreMessage(TCustomWinSocket *Socket)
{
BYTE            descriptor, supp_descriptor;
unsigned short  length;
int             ret1, ret2, ret3, ret4;

 if (Socket->ReceiveLength() == 0)
    return(ERRCORE_NOTHINGRECVD);

 ret1=ReceiveBufBytes(Socket, (char *)&descriptor, 1);
 SetDescriptor(descriptor);
 ret2=ReceiveBufBytes(Socket, (char *)&supp_descriptor, 1);
 SetSuppDescriptor(supp_descriptor);
 ret3=ReceiveBufBytes(Socket, (char *)&length, 2);
 SetLength(length);
 ret4=ReceiveBufBytes(Socket, GetBufPtr(), length);        // write directly into the buffer

 if ((ret1 == 0) || (ret2 == 0) || (ret3 == 0) || (ret4 == 0))
    return(ERRCORE_RECEIVEBUFBYTES);

 return(ERRCORE_NOERR);
}


// **************************************************************************
// Function:   ReceiveCoreMessage
// Purpose:    given a handle to a Socket stream that has been created for
//             a given Socket connection,
//             receives the content of a coremessage and sets the values
//             of the descriptor, the supplemental descriptor and the length
//             accordingly
//             use ParseMessage() to further process the coremessage
// Parameters: stream - an active stream
// Returns:    ERRCORE_NOERR success
//             ERRCORE_RECEIVEBUFBYTES on failure
// **************************************************************************
int COREMESSAGE::ReceiveCoreMessage(TWinSocketStream *stream)
{
BYTE            descriptor, supp_descriptor;
unsigned short  length;
int             bytesread;

 bytesread=ReceiveBufBytes(stream, (char *)&descriptor, 1);
 if (bytesread == 0) return(ERRCORE_RECEIVEBUFBYTES);
 SetDescriptor(descriptor);
 bytesread=ReceiveBufBytes(stream, (char *)&supp_descriptor, 1);
 if (bytesread == 0) return(ERRCORE_RECEIVEBUFBYTES);
 SetSuppDescriptor(supp_descriptor);
 bytesread=ReceiveBufBytes(stream, (char *)&length, 2);
 if (bytesread == 0) return(ERRCORE_RECEIVEBUFBYTES);
 SetLength(length);
 bytesread=ReceiveBufBytes(stream, GetBufPtr(), length);        // write directly into the buffer
 if (bytesread == 0) return(ERRCORE_RECEIVEBUFBYTES);

 return(ERRCORE_NOERR);
}


// **************************************************************************
// Function:   SendCoreMessage
// Purpose:    given a handle to an open non-blocking Socket connection,
//             sends a coremessage, including the appropriate header
//             values for the descriptor, the supplemental descriptor and the length
//             have to be specified accordingly
// Parameters: Socket - the non-blocking socket
// Returns:    ERRCORE_NOERR on success
//             ERRCORE_SENDBUFBYTES, if there was an error sending bytes out
//             ERRCORE_SOCKETNOTOPEN, if the socket connection was not open
// **************************************************************************
int COREMESSAGE::SendCoreMessage(TCustomWinSocket *Socket)
{
 if (!(Socket->Connected)) return(ERRCORE_SOCKETNOTOPEN);

 if (SendBufBytes(Socket, (char *)&descriptor, 1) <= 0) return(ERRCORE_SENDBUFBYTES);
 if (SendBufBytes(Socket, (char *)&supp_descriptor, 1) <= 0) return(ERRCORE_SENDBUFBYTES);
 if (SendBufBytes(Socket, (char *)&length, 2) <= 0) return(ERRCORE_SENDBUFBYTES);
 if (SendBufBytes(Socket, GetBufPtr(), length) <= 0) return(ERRCORE_SENDBUFBYTES);

 return(ERRCORE_NOERR);
}


// **************************************************************************
// Function:   SendCoreMessage
// Purpose:    given a handle to an open Socket stream,
//             sends a coremessage, including the appropriate header
//             values for the descriptor, the supplemental descriptor and the length
//             have to be specified accordingly
// Parameters: stream - the stream that has been created for a blocking socket connection
// Returns:    ERRCORE_NOERR on success
//             ERRCORE_WRITE on failure
// **************************************************************************
int COREMESSAGE::SendCoreMessage(TWinSocketStream *stream)
{
int bytes;

 try
  {
  bytes=stream->Write((char *)&descriptor, 1);
  if (bytes == 0) return(ERRCORE_WRITE);
  bytes=stream->Write((char *)&supp_descriptor, 1);
  if (bytes == 0) return(ERRCORE_WRITE);
  bytes=stream->Write((char *)&length, 2);
  if (bytes == 0) return(ERRCORE_WRITE);
  bytes=stream->Write(GetBufPtr(), length);
  if (bytes == 0) return(ERRCORE_WRITE);
  }
 catch( ESocketError& )
  {
  return(ERRCORE_WRITE);
  }

 return(ERRCORE_NOERR);
}


/* // **************************************************************************
// Function:   SendCoreMessage
// Purpose:    given a pointer to a CORECOMM object,
//             sends a coremessage, including the appropriate header
//             values for the descriptor, the supplemental descriptor and the length
//             have to be specified accordingly
// Parameters: corecomm - pointer to the communication object
// Returns:    ERRCORE_NOERR on success
//             ERRCORE_WRITE on failure
// **************************************************************************
int COREMESSAGE::SendCoreMessage(CORECOMM *corecomm)
{
TWinSocketStream        *pStream;
int bytes;

 stream=new TWinSocketStream(corecomm->ClientSocket, 5000);
 if (!stream) return(ERRCORE_WRITE);

 try
  {
  bytes=stream->Write((char *)&descriptor, 1);
  if (bytes == 0) return(ERRCORE_WRITE);
  bytes=stream->Write((char *)&supp_descriptor, 1);
  if (bytes == 0) return(ERRCORE_WRITE);
  bytes=stream->Write((char *)&length, 2);
  if (bytes == 0) return(ERRCORE_WRITE);
  bytes=stream->Write(GetBufPtr(), length);
  if (bytes == 0) return(ERRCORE_WRITE);
  }
 catch( TooGeneralCatch& )
  {
  delete stream;
  return(ERRCORE_WRITE);
  }

 delete stream;
 return(ERRCORE_NOERR);
}  */


// **************************************************************************
// Function:   ParseMessage
// Purpose:    After having received a coremessage, ParseMessage()
//             further processes the message. Depending on the type of the
//             coremessage, it processes the state, parameter, or status,
//             and initializes the respective object (status, param, state) in
//             the coremessage object
// Parameters: N/A
// Returns:    the type of the coremessage (as defined in UCoreMessage.h)
//             or COREMSG_NONE, if the type is not recognized
// **************************************************************************
int COREMESSAGE::ParseMessage()
{
 // the message is a status message
 if (descriptor == COREMSG_STATUS)
    {
    status.ParseStatus(buffer, length);         // parse the status line
    return(COREMSG_STATUS);
    }

 // the message is a parameter
 if (descriptor == COREMSG_PARAMETER)
    {
    param.ParseParameter(buffer, length);       // parse the parameter line
    return(COREMSG_PARAMETER);
    }

 // the message is a state
 if (descriptor == COREMSG_STATE)
    {
    state.ParseState(buffer, length);           // parse the state line
    return(COREMSG_STATE);
    }

 // the message is a SYSCMD
 if (descriptor == COREMSG_SYSCMD)
    {
    syscmd.ParseSysCmd(buffer, length);         // parse the system command
    return(COREMSG_SYSCMD);
    }

 // the message contains visualization data
 if (descriptor == COREMSG_DATA)
    {
    visualization.SetVisualizationType(GetSuppDescriptor());
    visualization.ParseVisualization(buffer, length);   // parse visualization data
    return(COREMSG_DATA);
    }

 // the message contains a state vector
 if (descriptor == COREMSG_STATEVECTOR)
    {
    return(COREMSG_STATEVECTOR);
    }

 return(COREMSG_NONE);
}



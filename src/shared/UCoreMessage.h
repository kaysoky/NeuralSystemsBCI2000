//---------------------------------------------------------------------------
#ifndef UCoreMessageH
#define UCoreMessageH

#include <ScktComp.hpp>

#define COREMESSAGE_MAXBUFFER   65536
#define COREMESSAGE_HEADERSIZE  3

#define COREMSG_ERROR           -1
#define COREMSG_NONE            0
#define COREMSG_STATUS          1
#define COREMSG_PARAMETER       2
#define COREMSG_STATE           3
#define COREMSG_DATA            4
#define COREMSG_STATEVECTOR     5
#define COREMSG_SYSCMD          6

#define ERRCORE_NOERR           0
#define ERRCORE_SENDBUFBYTES    1
#define ERRCORE_SOCKETNOTOPEN   2
#define ERRCORE_WRITE           3
#define ERRCORE_RECEIVEBUFBYTES 4
#define ERRCORE_NOTHINGRECVD    5

#include "USysCommand.h"
#include "UParameter.h"
#include "UState.h"
#include "UStatus.h"
#include "UGenericVisualization.h"
//---------------------------------------------------------------------------

class COREMESSAGE
{
private:	// User declarations
        char*   buffer;
        long    buffer_size;
        BYTE    descriptor;
        BYTE    supp_descriptor;
        unsigned short   length;
        int     SendBufBytes(TCustomWinSocket *Socket, const char *buf, int length);
        int     ReceiveBufBytes(TCustomWinSocket *Socket, char *buf, int length);
        int     ReceiveBufBytes(TWinSocketStream *stream, char *buf, int length);
public:		// User declarations
        COREMESSAGE::COREMESSAGE();
        ~COREMESSAGE();
        PARAM   param;
        STATE   state;
        STATUS  status;
        SYSCMD  syscmd;
        GenericVisualization    visualization;
        int     ParseMessage();
        void    SetDescriptor(BYTE descriptor);
        void    SetSuppDescriptor(BYTE newsuppdescriptor);
        BYTE    GetDescriptor() const;
        BYTE    GetSuppDescriptor() const;
        void    SetLength(int newlength);
        unsigned short GetLength() const;
        char    *GetBufPtr( int ); // when going to write into the buffer, you
                                   // must specify the amount of data to write.
        const char* GetBufPtr() const;
        int     ReceiveCoreMessage(TCustomWinSocket *Socket);
        int     ReceiveCoreMessage(TWinSocketStream *stream);
        int     SendCoreMessage(TCustomWinSocket *Socket);
        int     SendCoreMessage(TWinSocketStream *stream);
};

//---------------------------------------------------------------------------
#endif


// dtfun.h

#include <olmem.h>
#include <olerrors.h>
#include <oldaapi.h>
#include <stdio.h>

#include <scktcomp.hpp>


#define MAXCHANS     64
#define MAXPOINTS 0x7ffff
#define BUFNUM        10        // was 3 and 5

DECLARE_HANDLE(HDRVR);

class DTFUN
{
public:
        TEvent *bdone;
        int BufferCount;
        int BufferPtr;
        int ClockHz;
        char    BoardName[64];
        ECODE errc;
	int iMsg;
        short data[MAXPOINTS];
        HDASS lphDass;          //   Subsystem Handle
        int save_flag;
        char DatFile[80];
        int n_display;
        TCriticalSection *data_critsec;         // critical section for data FIFO

        __fastcall Start( void );
        __fastcall Stop( void );
        __fastcall Reset( void );
        __fastcall CleanUp( void );
        __fastcall Terminator( void );
        DTFUN::DTFUN( void );
        DTFUN::~DTFUN( void );
        void Save( void );
        void __fastcall SetWindow( HWND );
        void __fastcall SetFunction( void );
        void __fastcall Add_to_data( short *, ULNG );
	void  __fastcall InitBoard( void );
        ECODE __fastcall ConfigAD( UINT, UINT, DBL, int, DBL, DWORD);
private:


        HWND Ad_Win_Msg;        //   handle to window for A/D messages
        UINT LSize;             //   channel list size
        FILE *dfile;
        long d_count;           //  data counter

	ECODE status;
        LPSTR lpszName;
        HDRVR lphDev;           //   Board handle
        UINT uiDataFlow;

    //    DBL dfFreq;             //   sampling frequency
        LPARAM lParam;          //   user-defined message parameter
        DWORD dwSize;           //   size of data buffers
        UINT uiWinFlags;        //   windows memory allocation flags
        HBUF hbuf[BUFNUM];      //   buffer pointers
        int BufferSize;         //   buffer size

        UINT __fastcall SetChanType( UINT );
        UINT __fastcall SetChanList( UINT, DBL );
        DBL  __fastcall SetClock(int , DBL  );
        ECODE __fastcall SetWndHandle( void );
        ECODE __fastcall SetBuffers( DWORD );
}     ;

extern DTFUN dtfun;

__stdcall BufferDone( UINT, unsigned int, LPARAM );

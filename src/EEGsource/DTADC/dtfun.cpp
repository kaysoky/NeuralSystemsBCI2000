// dtadfun.cpp

#include <vcl.h>
#pragma hdrstop

#include <stdlib.h>

#include "dtfun.h"
// #include "MsgWin1.h" // we do not need this anymore; not notified by Windows messages ...
#include "UBCITime.h"

//---------------------------------------------------------------------------
DTFUN::DTFUN( void )
{
 data_critsec=new TCriticalSection();
}
//---------------------------------------------------------------------------

DTFUN::~DTFUN( void )
{
 if (data_critsec) delete data_critsec;
 data_critsec=NULL;
}

//---------------------------------------------------------------------------
__fastcall DTFUN::Start( void )
{
int     ret;
ECODE   status;

 status= olDaStart( lphDass );
 ret=1;
 if (status != OLNOERROR) ret=0;

 return(ret);
}

//----------------------------------------------------------------------------
__fastcall DTFUN::Stop( void )
{
        ECODE status;

        status= olDaAbort(lphDass);
        return( status );
}
//-----------------------------------------------------------------------------

__fastcall DTFUN::Reset( void )
{
        ECODE status;

        status= olDaReset( lphDass );
        return( status );
}

//-----------------------------------------------------------------------------

__fastcall DTFUN::CleanUp( void )
{
        int i;
        int bcount;

        status= olDaFlushBuffers( lphDass );

        bcount= BUFNUM;

        for(i=0;i<bcount;i++)
        {
                status= olDaGetBuffer( lphDass, &hbuf[i] );
                status= olDmFreeBuffer( hbuf[i] );
        }

        status= olDaReleaseDASS( lphDass );

        return( status );
}

//-----------------------------------------------------------------------------

__fastcall DTFUN::Terminator( void )
{
        ECODE status;
        status= olDaTerminate( lphDev );

        return( status );
}

//-----------------------------------------------------------------------------
void __fastcall DTFUN::SetWindow( HWND msgw )
{
        Ad_Win_Msg= msgw;
}

//---------------------------------------------------------------------------

void __fastcall DTFUN::InitBoard( void )
{
        UINT uiElement;
        LPUINT lpuiBits;

        lpszName= BoardName;  // "BCI_IN";
        lphDev = NULL;

        uiElement= 0;

        status= olDaInitialize( lpszName, &lphDev );

        status= olDaGetDASS( lphDev,
                             OLSS_AD,
                             uiElement,
                             &lphDass) ;

        uiDataFlow= OL_DF_CONTINUOUS;

        status= olDaSetDataFlow( lphDass, uiDataFlow );

        iMsg= status;

        status= olDaGetResolution( lphDass, lpuiBits );
        ADSize= (int)(*lpuiBits);

}

//--------------------------------------------------------------------

UINT __fastcall DTFUN::SetChanType( UINT ChanType )
{
       errc= olDaSetChannelType( lphDass, ChanType);
       return( errc );
}

UINT __fastcall DTFUN::SetChanList( UINT ListSize, DBL dGain )
{
     int count;

     LSize= ListSize;

     errc= olDaSetChannelListSize( lphDass, ListSize );

     for(count=0; count< (int)ListSize; count++)
     {
        status= olDaSetChannelListEntry( lphDass, count ,count );

        status= olDaSetGainListEntry( lphDass, count, dGain );
     }
     return( status );
}


DBL __fastcall DTFUN::SetClock(int ClockSource, DBL Freq )
{
        ECODE status;

        if(ClockSource == 0)
        {
                status= olDaSetClockSource( lphDass, OL_CLK_INTERNAL );
        }
        else
        {
                status= olDaSetClockSource( lphDass, OL_CLK_EXTERNAL );
        }

        status= olDaSetClockFrequency( lphDass, Freq );

        status= olDaConfig( lphDass );

        status= olDaGetClockFrequency( lphDass, &Freq );

        return( Freq );
}

ECODE __fastcall DTFUN::SetWndHandle( void )
{

        errc= olDaSetWndHandle( lphDass, Ad_Win_Msg, lParam );
        return( errc );
}

ECODE __fastcall DTFUN::SetBuffers( DWORD BufSize )
{
        int i;
        ECODE retval= 0;

        status= olDaSetWrapMode( lphDass, OL_WRP_MULTIPLE  );

        dwSize= BufSize;

        for(i=0;i<BUFNUM;i++)
        {
                retval+= olDmAllocBuffer( 0, dwSize, &hbuf[i] );
                retval+= olDaPutBuffer( lphDass, hbuf[i] );
        }

        retval+= olDaConfig( lphDass );

        d_count= 0;   // initialize data counter
        BufferCount= 0;
        BufferPtr= 0;

        return( status );
}

//----------------------------------------------------------------

void __fastcall DTFUN::Add_to_data(short lphBuf[], ULNG samples)
{
        unsigned i;
        unsigned short bt,dt;

        for(i=0;i<samples;i++)
        {
                if (ADSize == 16) data[BufferPtr]= (short)(lphBuf[i]-32678);
                else              data[BufferPtr]= (short)(lphBuf[i]-2048) * 16;
                BufferPtr++;
        }
        BufferCount++;
}


void __fastcall DTFUN::SetFunction(  void )
{
OLNOTIFYPROC    lpfnNotifyProc;
ECODE           errc;

 lpfnNotifyProc= &BufferDone;
 errc= olDaSetNotificationProcedure( lphDass, lpfnNotifyProc, lParam );
 bdone= new TEvent(NULL,false,false,"");
}

//----------------------------------------------------------------


// this function is called directly by the driver whenever data is ready
// there is an anomaly in the (current version of the) Data Translation driver
// that causes this function to be called unevenly spaced in time
// this only happens under certain circumstances (i.e., number of channels,
// SampleBlockSize, and sampling frequency)
__stdcall BufferDone( UINT uiMsg, unsigned int Dass, LPARAM lParam )
{
ECODE   status;
int     arrays;
int     i;
short   *buffer;
HBUF    hBuf;
LPHBUF  lphBuf;
ULNG    samples;
HDASS   lphDass;

 hBuf= NULL;
 lphBuf= NULL;
 lphDass= (void *)Dass;

 status= olDaGetBuffer(lphDass, &hBuf);

 if ( hBuf != NULL )
    {
    status= olDmGetMaxSamples(hBuf,&samples);
    status= olDmGetBufferPtr( hBuf, (LPVOID FAR*)&lphBuf );
    status= olDaPutBuffer(lphDass, hBuf);

    buffer= (short *)lphBuf;

    dtfun.data_critsec->Acquire();
    dtfun.Add_to_data( buffer, samples );  // add data to FIFO
    dtfun.data_critsec->Release();
    }

 // notify ADReadDataBlock() that data is here
 dtfun.bdone->SetEvent();
 return(0);
}


ECODE __fastcall DTFUN::ConfigAD( UINT ChanType,
                                 UINT ListSize,
                                 DBL  Gain,
                                 int  ClockSource,
                                 DBL  Freq,
                                 DWORD BufSize   )
{
        ECODE result= 0;
        BufferSize= (int)BufSize;

        result+= SetChanType( ChanType );
        result+= SetChanList( ListSize,Gain );
        Freq= SetClock(ClockSource,Freq );
        result+= SetWndHandle();
        result+= SetBuffers( BufSize );

        ClockHz= Freq;

        return( result );
}

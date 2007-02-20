/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef READBUFFER_H
#define READBUFFER_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the READBUF_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// READBUF_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.



#ifdef READBUF_EXPORTS
#define READBUF_API __declspec(dllexport)
#else
#define READBUF_API __declspec(dllimport)
#endif

/*   Example:
// This class is exported from the readbuf.dll
class READBUF_API CReadbuf {
public:
	CReadbuf(void);
	// TODO: add your methods here.
};

extern READBUF_API int nReadbuf;

READBUF_API int fnReadbuf(void);
*/
//=====================================================


class  AnySampleArray
{

public:
	long nItems;
	long ItemSize;
	unsigned long TimeStamp;
	char * pData;
public:

	AnySampleArray(char * pCA)
	{
		long * pLP= (long*)pCA;
		nItems= pLP[0];
		ItemSize= pLP[1];
		TimeStamp= (unsigned long)pLP[2];
		pData=pCA+sizeof(nItems)+sizeof(ItemSize)+sizeof(TimeStamp);
	}; //

		
	long & operator[] (long N) //safe, wraps around
	{		
		return *(long *)&pData[(N*ItemSize)% (nItems*ItemSize)];  
	};
};


template <class T, short SZ>
class  SampleArray
{
public:
	long nItems;
	long ItemSize;
	long TimeStamp;
private:

	T Buf[SZ];
	long nMaxItems;
public:

	SampleArray():nItems(SZ),nMaxItems(SZ),TimeStamp(0) {ItemSize=sizeof(T);memset(Buf, 0, ItemSize*SZ);}; //

	T & operator[] (unsigned short N) //safe, wraps around
	{		
		return Buf[N%nMaxItems];
	};
  
	SampleArray(SampleArray<T,SZ> & C )
	{
		ItemSize=sizeof(T);
		short N = nMaxItems < C.nItems ? nMaxItems : C.nItems;
		memcpy(Buf,C.Buf,ItemSize*N);
	};

    SampleArray<T,SZ> & operator= (const AnySampleArray & C )
    {
		ItemSize= C.ItemSize;
		short N = nMaxItems < C.nItems ? nMaxItems : C.nItems;
		nItems = N;
		memcpy(Buf,C.pData,ItemSize*N);
		return *this;
    };

//	friend ostream & operator<< (ostream & S,SampleArray<T,SZ> & C);

};



typedef SampleArray<long,(290+1)> MaxSample;

//#define  SB_CopyItemT0  2L //   1 00001200 ?SB_CopyItemT0@@YAXAAV?$SampleArray@J$0BCD@@@@Z


READBUF_API		BOOL SB_PutItem(AnySampleArray * lpAnyCArray);  // returns FALSE if BufferFull
READBUF_API		BOOL SB_pGetItemPchar(char * *lpVoid, int index);	 // returns FALSE if BufferEmpty
READBUF_API		void SB_CopyItem(MaxSample & ThisSample,int TailIndex) ;
//READBUF_API		void  SB_CopyItemT0(MaxSample & ThisSample);
READBUF_API		float SB_GetBinwidth();
READBUF_API		int   SB_GetChRange();
READBUF_API		void  SB_SubtractBl(MaxSample & ThisSample);
READBUF_API		BOOL SB_pGetItemPtr(char * *lpVoid, int index);	 // returns FALSE if BufferEmpty
READBUF_API		int SB_GetItems();
READBUF_API		int SB_GetHead() ;
READBUF_API		int SB_GetTail(int tn);
READBUF_API		int SB_GetLastHead();
READBUF_API		int SB_GetBinwidthUs();
READBUF_API		void SB_SetBinwidthUs(long newUs);
READBUF_API		void SB_Reset(int tn);

//---------------------------------------------------------------
//
//     typedefs for runtime linking:
//      (with LoadLibrary and GetProcAddress)
//
//
typedef BOOL  (*SB_PutItem_PTR)(AnySampleArray * lpAnyCArray);  // returns FALSE if BufferFull
typedef BOOL  (*SB_pGetItemPchar_PTR)(char * *lpVoid, int index);	 // returns FALSE if BufferEmpty
typedef void  (*SB_CopyItem_PTR)(MaxSample & ThisSample,int TailIndex);
typedef float (*SB_GetBinwidth_PTR)();
typedef int   (*SB_GetChRange_PTR)();
typedef void  (*SB_SubtractBl_PTR)(MaxSample & ThisSample);
typedef BOOL  (*SB_pGetItemPtr_PTR)(char * *lpVoid, int index);	 // returns FALSE if BufferEmpty
typedef int   (*SB_GetItems_PTR)();
typedef int   (*SB_GetHead_PTR)() ;
typedef int   (*SB_GetTail_PTR)(int tn);
typedef int   (*SB_GetLastHead_PTR)();
typedef int   (*SB_GetBinwidthUs_PTR)();
typedef void  (*SB_SetBinwidthUs_PTR)(long newUs);
typedef void  (*SB_Reset_PTR)(int tn);


//extern READBUF_API		CRingBuffer <MaxSample> RingBufferNI;//(4096*2,"C:/BIOSEMI_"); //(4096);  //

//-------------------------------------------------------
//
//      Output by DUMPBIN:
//

//#define  1    0 00021808 ?RingBufferNI@@3V?$CRingBuffer@V?$SampleArray@J$0BCD@@@@@A


#define  nSB_PutItem       (LPCSTR)1L   //    9 00001130 ?SB_PutItem@@YAHPAVAnySampleArray@@@Z
#define  nSB_pGetItemPchar (LPCSTR)2L   //    D 00001170 ?SB_pGetItemPchar@@YAHPAPADH@Z
#define  nSB_CopyItem      (LPCSTR)3L   // for calls to GetProcAddress
#define  nSB_GetBinwidth   (LPCSTR)4L   //   2 00001250 ?SB_GetBinwidth@@YAMXZ
#define  nSB_GetChRange    (LPCSTR)5L   //    4 000012C0 ?SB_GetChRange@@YAHXZ
#define  nSB_SubtractBl    (LPCSTR)6L   //    C 00001310 ?SB_SubtractBl@@YAXAAV?$SampleArray@J$0BCD@@@@Z
#define  nSB_pGetItemPtr   (LPCSTR)7L   //    E 000011B0 ?SB_pGetItemPtr@@YAHPAPADH@Z
#define  nSB_GetItems      (LPCSTR)8L   //    6 000013F0 ?SB_GetItems@@YAHXZ
#define  nSB_GetHead       (LPCSTR)9L   //    5 00001420 ?SB_GetHead@@YAHXZ
#define  nSB_GetTail       (LPCSTR)10L   //    8 00001480 ?SB_GetTail@@YAHH@Z
#define  nSB_GetLastHead   (LPCSTR)11L   //    7 00001450 ?SB_GetLastHead@@YAHXZ
#define  nSB_GetBinwidthUs (LPCSTR)12L   //    3 000014B0 ?SB_GetBinwidthUs@@YAHXZ
#define  nSB_SetBinwidthUs (LPCSTR)13L   //    B 00001290 ?SB_SetBinwidthUs@@YAXJ@Z
#define  nSB_Reset         (LPCSTR)14L   //    A 000014E0 ?SB_Reset@@YAXH@Z

/*   EXPORTS in .def:

SB_PutItem			@1	
SB_pGetItemPchar	@2
SB_CopyItem 		@3
SB_GetBinwidth		@4
SB_GetChRange		@5
SB_SubtractBl		@6
SB_pGetItemPtr		@7
SB_GetItems			@8
SB_GetHead			@9
SB_GetTail			@10
SB_GetLastHead		@11
SB_GetBinwidthUs	@12
SB_SetBinwidthUs	@13
SB_Reset			@14	
*/
#endif

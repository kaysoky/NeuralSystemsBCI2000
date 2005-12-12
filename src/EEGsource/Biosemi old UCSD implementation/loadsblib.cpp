#include <vcl.h>  // for BC++
//#include "stdafx.h" // for VC++

#include "loadsblib.h"
SB_PutItem_PTR          pSB_PutItem      ;
SB_pGetItemPchar_PTR    pSB_pGetItemPchar;
SB_CopyItem_PTR         pSB_CopyItem     ;
SB_GetBinwidth_PTR      pSB_GetBinwidth  ;
SB_GetChRange_PTR       pSB_GetChRange   ;
SB_SubtractBl_PTR       pSB_SubtractBl   ;
SB_pGetItemPtr_PTR      pSB_pGetItemPtr  ;
SB_GetItems_PTR         pSB_GetItems     ;
SB_GetHead_PTR          pSB_GetHead      ;
SB_GetTail_PTR          pSB_GetTail      ;
SB_GetLastHead_PTR      pSB_GetLastHead  ;
SB_GetBinwidthUs_PTR    pSB_GetBinwidthUs;
SB_SetBinwidthUs_PTR    pSB_SetBinwidthUs;
SB_Reset_PTR            pSB_Reset        ;

HINSTANCE hDllSB;

int Init_SB_Functions()
{
        hDllSB=LoadLibrary("samplebuf.dll");
        if (hDllSB)
        {
                pSB_PutItem      =(SB_PutItem_PTR          )GetProcAddress(hDllSB,nSB_PutItem       ); if (pSB_PutItem==NULL) return 0;
                pSB_pGetItemPchar=(SB_pGetItemPchar_PTR    )GetProcAddress(hDllSB,nSB_pGetItemPchar ); if (pSB_pGetItemPchar==NULL) return 0;
                pSB_CopyItem     =(SB_CopyItem_PTR         )GetProcAddress(hDllSB,nSB_CopyItem      ); if (pSB_CopyItem     ==NULL) return 0;
                pSB_GetBinwidth  =(SB_GetBinwidth_PTR      )GetProcAddress(hDllSB,nSB_GetBinwidth   ); if (pSB_GetBinwidth  ==NULL) return 0;
                pSB_GetChRange   =(SB_GetChRange_PTR       )GetProcAddress(hDllSB,nSB_GetChRange    ); if (pSB_GetChRange   ==NULL) return 0;
                pSB_SubtractBl   =(SB_SubtractBl_PTR       )GetProcAddress(hDllSB,nSB_SubtractBl    ); if (pSB_SubtractBl   ==NULL) return 0;
                pSB_pGetItemPtr  =(SB_pGetItemPtr_PTR      )GetProcAddress(hDllSB,nSB_pGetItemPtr   ); if (pSB_pGetItemPtr  ==NULL) return 0;
                pSB_GetItems     =(SB_GetItems_PTR         )GetProcAddress(hDllSB,nSB_GetItems      ); if (pSB_GetItems     ==NULL) return 0;
                pSB_GetHead      =(SB_GetHead_PTR          )GetProcAddress(hDllSB,nSB_GetHead       ); if (pSB_GetHead      ==NULL) return 0;
                pSB_GetTail      =(SB_GetTail_PTR          )GetProcAddress(hDllSB,nSB_GetTail       ); if (pSB_GetTail      ==NULL) return 0;
                pSB_GetLastHead  =(SB_GetLastHead_PTR      )GetProcAddress(hDllSB,nSB_GetLastHead   ); if (pSB_GetLastHead  ==NULL) return 0;
                pSB_GetBinwidthUs=(SB_GetBinwidthUs_PTR    )GetProcAddress(hDllSB,nSB_GetBinwidthUs ); if (pSB_GetBinwidthUs==NULL) return 0;
                pSB_SetBinwidthUs=(SB_SetBinwidthUs_PTR    )GetProcAddress(hDllSB,nSB_SetBinwidthUs ); if (pSB_SetBinwidthUs==NULL) return 0;
                pSB_Reset        =(SB_Reset_PTR            )GetProcAddress(hDllSB,nSB_Reset         ); if (pSB_Reset        ==NULL) return 0;
                return 1;
        }
        return 0;
}

 
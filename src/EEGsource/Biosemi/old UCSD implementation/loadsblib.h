#ifndef LOADSBLIB_H
#define LOADSBLIB_H

#include "samplbuf.h"
extern SB_PutItem_PTR          pSB_PutItem      ;
extern SB_pGetItemPchar_PTR    pSB_pGetItemPchar;
extern SB_CopyItem_PTR		   pSB_CopyItem   ;
extern SB_GetBinwidth_PTR      pSB_GetBinwidth  ;
extern SB_GetChRange_PTR       pSB_GetChRange   ;
extern SB_SubtractBl_PTR       pSB_SubtractBl   ;
extern SB_pGetItemPtr_PTR      pSB_pGetItemPtr  ;
extern SB_GetItems_PTR         pSB_GetItems     ;
extern SB_GetHead_PTR          pSB_GetHead      ;
extern SB_GetTail_PTR          pSB_GetTail      ;
extern SB_GetLastHead_PTR      pSB_GetLastHead  ;
extern SB_GetBinwidthUs_PTR    pSB_GetBinwidthUs;
extern SB_SetBinwidthUs_PTR    pSB_SetBinwidthUs;
extern SB_Reset_PTR            pSB_Reset        ;

extern HINSTANCE hDllSB;
int Init_SB_Functions();

#endif
/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file mbox.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#ifndef __mbox_H_wg1E2p0E_lhlC_HT3B_swKo_uiHxZHT0LHTG__
#define __mbox_H_wg1E2p0E_lhlC_HT3B_swKo_uiHxZHT0LHTG__

#ifdef __cplusplus
extern "C" {
#endif


#include "smf_pmsgq.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
struct mbox;
typedef void (*CB_MBOX_DESTROY)(struct mbox *pMbox);
/**
 *  message box
 */
typedef struct mbox
{
    smf_pmsgq_base_t    base;

    long                ref_cnt;
    long                frome;

    CB_MBOX_DESTROY     cbMboxDestroy;

    union {
        struct {
            void        *pAddr;
        } def;
    } data;

} mbox_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif

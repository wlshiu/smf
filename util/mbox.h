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

typedef int (*CB_MBOX_MUTEX_INIT)(struct mbox *pMbox, void *pExtra_data);
typedef int (*CB_MBOX_MUTEX_DEINIT)(struct mbox *pMbox, void *pExtra_data);
typedef int (*CB_MBOX_MUTEX_LOCK)(struct mbox *pMbox, void *pExtra_data);
typedef int (*CB_MBOX_MUTEX_UNLOCK)(struct mbox *pMbox, void *pExtra_data);

typedef void (*CB_MBOX_DESTROY)(struct mbox **ppMbox, void *pExtra_data);
/**
 *  message box
 */
typedef struct mbox
{
    smf_pmsgq_base_t    base;

    long                ref_cnt;
    long                from;

    CB_MBOX_MUTEX_INIT      cb_mbox_mutex_init;
    CB_MBOX_MUTEX_DEINIT    cb_mbox_mutex_deinit;
    CB_MBOX_MUTEX_LOCK      cb_mbox_mutex_lock;
    CB_MBOX_MUTEX_UNLOCK    cb_mbox_mutex_unlock;

    CB_MBOX_DESTROY     cb_mbox_destroy;

    union {
        struct {
            void        *pAddr;
        } def;
    } data;

    // user attaches private info
    void                *pTunnel_info[2];

    // for share memory case if user wants
    long                is_used;

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

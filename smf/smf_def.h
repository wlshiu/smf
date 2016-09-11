/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_def.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#ifndef __smf_def_H_wiFKp8GL_lBc1_HETp_sa3o_u4pACSBYvBvb__
#define __smf_def_H_wiFKp8GL_lBc1_HETp_sa3o_u4pACSBYvBvb__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#include "pthread.h"

// mutex api
#define smf_mutex_t                          pthread_mutex_t
#define smf_mutex_init(pMutex, pAttr)        pthread_mutex_init((pMutex), pAttr)
#define smf_mutex_destroy(pMutex)            pthread_mutex_destroy((pMutex))
#define smf_mutex_lock(pMutex)               pthread_mutex_lock((pMutex))
#define smf_mutex_unlock(pMutex)             pthread_mutex_unlock((pMutex))



#define toStr(x)            #x

/**
 * handle check function
 */
#define _smf_verify_handle(handle, err_code)                        \
            do{ if((handle)==NULL){                                 \
                log(LOG_MISC, "%s= Null !\n", toStr(handle));       \
                return err_code;}                                   \
            }while(0)

#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (unsigned long)&(((type *)0)->member)
#endif

#ifndef STRUCTURE_POINTER
    #define STRUCTURE_POINTER(type, ptr, member)    (type*)((unsigned long)ptr - MEMBER_OFFSET(type, member))
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

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

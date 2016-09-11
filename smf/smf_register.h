/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_register.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#ifndef __smf_register_H_wmeZdoP2_liwb_HOt8_sMpj_u0r7jBmlIsrU__
#define __smf_register_H_wmeZdoP2_liwb_HOt8_sMpj_u0r7jBmlIsrU__

#ifdef __cplusplus
extern "C" {
#endif


#include "smf_elem_desc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define SMF_REGISTER_ELEMENT(descriptor, ppFirstDesc)     \
    do{ extern smf_elem_desc_t  descriptor;               \
        smf_elem_desc_t         **p;                      \
        p = (ppFirstDesc);                                \
        while(*p)   p = &(*p)->next;                      \
        *p = &descriptor;                                 \
        descriptor.next = 0;                              \
    }while(0)


//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static inline void
_smf_register_element(smf_elem_desc_t  **ppFirstDesc)
{
    static int  bInitialized = 0;
    if( bInitialized )
        return;

    SMF_REGISTER_ELEMENT(g_elem_test_desc, ppFirstDesc);


    // reserve for user implement by self
    SMF_REGISTER_ELEMENT(g_elem_customization_desc, ppFirstDesc);

    bInitialized = 1;
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif

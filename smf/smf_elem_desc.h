/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_elem_desc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#ifndef __smf_elem_desc_H_wvQZoeyi_lkNH_HnHl_sC2u_uY3HYXxDJxIr__
#define __smf_elem_desc_H_wvQZoeyi_lkNH_HnHl_sC2u_uY3HYXxDJxIr__

#ifdef __cplusplus
extern "C" {
#endif

#include "smf_ctrl.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
struct smf_elem_desc;

/**
 *  linker of smf for one to many
 */
typedef struct smf_linker
{
    struct smf_linker       *next, *prev;
    struct smf_elem_desc    *pElem_desc;

} smf_linker_t;

/**
 *  descriptor of a smf element
 */
typedef struct smf_elem_desc
{
    struct smf_elem_desc    *next, *prev;

    // linker
    smf_linker_t            *pLinker_head;
    smf_linker_t            *pLinker_cur;

    smf_elem_priv_info_t    elem_priv;

} smf_elem_desc_t;
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

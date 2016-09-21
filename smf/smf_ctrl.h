/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_ctrl.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#ifndef __smf_ctrl_H_wgyIoykp_l2tw_HqMo_sL0x_uZHxpuCDhnpz__
#define __smf_ctrl_H_wgyIoykp_l2tw_HqMo_sL0x_uZHxpuCDhnpz__

#ifdef __cplusplus
extern "C" {
#endif

#include "smf_err.h"

/**
 *  architecture:
 *
 *          element_A  <-- add --> element_B
 *               x
 *              / \
 *             /   \ bind
 *            /     \
 *      element_1   element_2
 *
 */
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum smf_elem_id
{
    SMF_ELEM_TEST,
    SMF_ELEM_CUSTOMIZATION,

} smf_elem_id_t;

/**
 *  order
 */
typedef enum smf_elem_order
{
    SMF_ELEM_ORDER_FORWARD,
    SMF_ELEM_ORDER_BACKWARD,

} smf_elem_order_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  arguments
 */
typedef struct smf_args
{
    union {
        unsigned int        u32_value;
        unsigned long long  u64_value;
        void                *pAddr;
    } arg[4];
} smf_args_t;

/**
 *  initial info
 */
typedef struct smf_init_info
{
    void        *pTunnel_info;

} smf_init_info_t;


typedef struct smf_elem_priv_info
{
    unsigned long       uid;
    char                *name;

    // for user use
    void                *pTunnel_info[4];

    // inint method
    smf_err_t (*cbInit)(struct smf_elem_priv_info *pElem_prev);

    // deinit method
    smf_err_t (*cbDeInit)(struct smf_elem_priv_info *pElem_prev);

    /**
     *  If user want to handle something after de-initialize. e.g. release something which is attached at pTunnel_info[]
     */
    void (*cbAfterDeInit)(struct smf_elem_priv_info *pElem_prev);

    /**
     *  Receive message:
     *      It should not be braked
     */
    smf_err_t (*cbRecvMsg)(struct smf_elem_priv_info *pElem_prev, void *pMsg, smf_args_t *pShare2Next);

} smf_elem_priv_info_t;

/**
 *  handle of service management framework
 */
typedef struct smf_handle
{
    unsigned int        reversed;

} smf_handle_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
smf_err_t
Smf_Create(
    smf_handle_t        **ppHSmf,
    smf_init_info_t     *pInit_info);


smf_err_t
Smf_Destroy(
    smf_handle_t        **ppHSmf);


smf_err_t
Smf_Elem_New(
    smf_handle_t            *pHSmf,
    smf_elem_id_t           ElemType,
    smf_elem_priv_info_t    **ppElem_info);


smf_err_t
Smf_Elem_Del(
    smf_handle_t        *pHSmf,
    unsigned long       uid,
    void                *pExtra_data);


smf_err_t
Smf_Elem_Bind(
    smf_handle_t            *pHSmf,
    smf_elem_priv_info_t    *pElem_cur,
    smf_elem_priv_info_t    *pElem_next);


smf_err_t
Smf_Elem_Add(
    smf_handle_t            *pHSmf,
    smf_elem_priv_info_t    *pElem);


smf_err_t
Smf_Start(
    smf_handle_t        *pHSmf,
    smf_elem_order_t    ordert,
    void                *pExtra_data);


smf_err_t
Smf_Stop(
    smf_handle_t        *pHSmf,
    smf_elem_order_t    order,
    void                *pExtra_data);


smf_err_t
Smf_Send_Msg(
    smf_handle_t        *pHSmf,
    smf_elem_order_t    order,
    void                *pMsg);


#ifdef __cplusplus
}
#endif

#endif

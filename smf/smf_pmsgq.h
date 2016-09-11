/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_pmsgq.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#ifndef __smf_pmsgq_H_wQdPcgka_lo88_Hcir_saQU_uBfKh4W89vmw__
#define __smf_pmsgq_H_wQdPcgka_lo88_Hcir_saQU_uBfKh4W89vmw__

#ifdef __cplusplus
extern "C" {
#endif

#include "smf_err.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  priority type
 */
typedef struct smf_pmsgq_priority
{
    union {
        void                *pAddr;
        unsigned int        u32_value;
        unsigned long long  u64_value;
    } u;
} smf_pmsgq_priority_t;

/** callback functions to get/set/compare the priority of an element */
typedef smf_pmsgq_priority_t* (*CB_PRIORITY_GET)(void *pNode);
typedef void (*CB_PRIORITY_SET)(void *pNode, smf_pmsgq_priority_t *pNode_priority);

/**
 *  compare 2 nodes priority,
 *  return 'true'   => change nodes
 *         'false'  => keep state
 */
typedef long (*CB_PRIORITY_CMP)(smf_pmsgq_priority_t *pNode_next, smf_pmsgq_priority_t *pNode_cur);


/** callback functions to get/set the position of an element in internal node list */
typedef long (*CB_POSITION_GET)(void *pNode);
typedef void (*CB_POSITION_SET)(void *pNode, long index);


/** debug callback function to print a entry */
typedef void (*CB_PRINT_ENTRY)(void *pOut_dev, void *pNode, void *pExtra);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  base info of PMsg
 *      All Msg_node MUST Inherited from smf_pmsgq_base_t
 */
typedef struct smf_pmsgq_base
{
    smf_pmsgq_priority_t    priority;
    long                    position;
} smf_pmsgq_base_t;


/**
 *  init info
 */
typedef struct smf_pmsgq_init_info
{
    int                 amount_nodes;

    CB_PRIORITY_GET     cbPriGet;
    CB_PRIORITY_SET     cbPriSet;
    CB_PRIORITY_CMP     cbPriCmp;

    CB_POSITION_GET     cbPosGet;
    CB_POSITION_SET     cbPosSet;

} smf_pmsgq_init_info_t;


/**
 *  handle of priority message queue
 */
typedef struct smf_pmsgq_handle
{
    long        remain_num;
} smf_pmsgq_handle_t;
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
SmfPMsgq_Create(
    smf_pmsgq_handle_t       **ppHPMsg,
    smf_pmsgq_init_info_t    *pInit_info);


smf_err_t
SmfPMsgq_Destroy(smf_pmsgq_handle_t  **ppHPMsg);


smf_err_t
SmfPMsgq_Node_Push(
    smf_pmsgq_handle_t       *pHPMsg,
    void                    *pNode);


smf_err_t
SmfPMsgq_Node_Pop(
    smf_pmsgq_handle_t   *pHPMsg,
    void                 **ppNode);


smf_err_t
SmfPMsgq_Node_Change_Priority(
    smf_pmsgq_handle_t       *pHPMsg,
    smf_pmsgq_priority_t     *pNewPri,
    void                     *pNode);


smf_err_t
SmfPMsgq_Node_Peek(
    smf_pmsgq_handle_t   *pHPMsg,
    void                 **ppNode);


smf_err_t
SmfPMsgq_Node_Remove(
    smf_pmsgq_handle_t   *pHPMsg,
    void                 *pNode);


smf_err_t
SmfPMsgq_Print_All(
    smf_pmsgq_handle_t   *pHPMsg,
    void                 *pOut_dev,
    void                 *pExtra,
    CB_PRINT_ENTRY       cbPrint);



#ifdef __cplusplus
}
#endif

#endif

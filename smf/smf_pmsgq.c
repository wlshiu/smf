/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_pmsgq.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/09/11
 * @license
 * @description
 */

#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "smf_def.h"
#include "smf_pmsgq.h"
#include "mleak_check.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define LEFT(x)         ((x) << 1)
#define RIGHT(x)        (((x) << 1) + 1)
#define PARENT(x)       ((x) >> 1)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct pmsgq_dev
{
    smf_pmsgq_handle_t       hPMsg;

    smf_mutex_t             mutex_pmsgq;

    long                    node_cnt;
    long                    max_nodes;

    CB_PRIORITY_GET         cbPriGet;
    CB_PRIORITY_SET         cbPriSet;
    CB_PRIORITY_CMP         cbPriCmp;

    CB_POSITION_GET         cbPosGet;
    CB_POSITION_SET         cbPosSet;

    void                    **ppNode_list;

} pmsgq_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_bubble_up(
    pmsgq_dev_t  *pDev,
    long         idx)
{
    long                    parent_idx = 0l;
    void                    **ppNode_list = pDev->ppNode_list;
    void                    *pCur_node = 0;
    smf_pmsgq_priority_t    *pCur_node_pri = 0;
    CB_PRIORITY_GET         cbPriGet = pDev->cbPriGet;
    CB_PRIORITY_CMP         cbPriCmp = pDev->cbPriCmp;
    CB_POSITION_SET         cbPosSet = pDev->cbPosSet;

    pCur_node     = ppNode_list[idx];
    pCur_node_pri = cbPriGet(pCur_node);

    for(parent_idx = PARENT(idx);
        (idx > 1) && cbPriCmp(cbPriGet(ppNode_list[parent_idx]), pCur_node_pri);
        idx = parent_idx, parent_idx = PARENT(idx))
    {
        ppNode_list[idx] = ppNode_list[parent_idx];
        cbPosSet(ppNode_list[idx], (int)idx);
    }

    ppNode_list[idx] = pCur_node;
    cbPosSet(pCur_node, (int)idx);

    return;
}

static long
_get_child_idx(
    pmsgq_dev_t  *pDev,
    long         idx)
{
    long                child_idx = LEFT(idx);
    void                **ppNode_list = pDev->ppNode_list;
    CB_PRIORITY_GET     cbPriGet = pDev->cbPriGet;
    CB_PRIORITY_CMP     cbPriCmp = pDev->cbPriCmp;

    if( child_idx >= pDev->node_cnt )
        return 0l;

    // choice left or right child node
    if( (child_idx + 1) < pDev->node_cnt &&
        cbPriCmp(cbPriGet(ppNode_list[child_idx]), cbPriGet(ppNode_list[child_idx+1])) )
        child_idx++; // select right child node

    return child_idx;
}

static void
_percolate_down(
    pmsgq_dev_t  *pDev,
    long         idx)
{
    long                    child_idx = 0l;
    void                    **ppNode_list = pDev->ppNode_list;
    void                    *pCur_node = 0;
    smf_pmsgq_priority_t    *pCur_node_pri = 0;
    CB_PRIORITY_GET         cbPriGet = pDev->cbPriGet;
    CB_PRIORITY_CMP         cbPriCmp = pDev->cbPriCmp;
    CB_POSITION_SET         cbPosSet = pDev->cbPosSet;

    pCur_node     = ppNode_list[idx];
    pCur_node_pri = cbPriGet(pCur_node);

    while( (child_idx = _get_child_idx(pDev, idx)) &&
            cbPriCmp(pCur_node_pri, cbPriGet(ppNode_list[child_idx])) )
    {
        ppNode_list[idx] = ppNode_list[child_idx];
        cbPosSet(ppNode_list[idx], (int)idx);

        idx = child_idx;
    }

    ppNode_list[idx] = pCur_node;
    cbPosSet(pCur_node, idx);

    return;
}

static void
_print_set_pos(void *pNode, long idx)
{
    /* when print, do nothing */
    return;
}


static void
_print_set_pri(void *pNode, smf_pmsgq_priority_t *pPri)
{
    /* when print, do nothing */
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
smf_err_t
SmfPMsgq_Create(
    smf_pmsgq_handle_t      **ppHPMsg,
    smf_pmsgq_init_info_t   *pInit_info)
{
    smf_err_t       rval = SMF_ERR_OK;
    pmsgq_dev_t     *pDev = 0;

    do {
        if( !ppHPMsg || (*ppHPMsg) || !pInit_info )
        {
            log_err("%s", "input null pointer\n");
            rval = SMF_ERR_INVALID_PARAM;
            break;
        }

        if( !pInit_info->cbPriGet || !pInit_info->cbPriSet || !pInit_info->cbPriCmp ||
            !pInit_info->cbPosGet || !pInit_info->cbPosSet )
        {
            log_err("%s", "callback can't be null \n");
            rval = SMF_ERR_INVALID_PARAM;
            break;
        }

        //--------------------------
        // malloc handle
        if( !(pDev = malloc(sizeof(pmsgq_dev_t))) )
        {
            log_err("malloc fail (size= %d)\n", sizeof(pmsgq_dev_t));
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pDev, 0x0, sizeof(pmsgq_dev_t));

        //-----------------------------------
        if( smf_mutex_init(&pDev->mutex_pmsgq, 0) )
        {
            log_err("%s", "create mutex_pmsgq Fail !\n");
            rval = SMF_ERR_UNKNOWN;
            break;
        }

        // element 0 isn't used for mapping indxe and count.
        pDev->max_nodes = pInit_info->amount_nodes + 1;
        pDev->node_cnt  = 1;

        pDev->cbPriGet = pInit_info->cbPriGet;
        pDev->cbPriSet = pInit_info->cbPriSet;
        pDev->cbPriCmp = pInit_info->cbPriCmp;
        pDev->cbPosGet = pInit_info->cbPosGet;
        pDev->cbPosSet = pInit_info->cbPosSet;

        if( !(pDev->ppNode_list = malloc(sizeof(void*) * pDev->max_nodes)) )
        {
            log_err("malloc fail (size= %ld)\n", sizeof(void*) * pDev->max_nodes);
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pDev->ppNode_list, 0x0, sizeof(void*) * pDev->max_nodes);

        pDev->hPMsg.remain_num = pDev->node_cnt - 1;
        //------------------------
        *ppHPMsg = &pDev->hPMsg;

    } while(0);

    if( rval )
    {
        smf_pmsgq_handle_t  *pHPMsg = &pDev->hPMsg;
        SmfPMsgq_Destroy(&pHPMsg);
    }

    return rval;
}


smf_err_t
SmfPMsgq_Destroy(smf_pmsgq_handle_t  **ppHPMsg)
{
    smf_err_t      rval = SMF_ERR_OK;

    do {
        pmsgq_dev_t         *pDev = 0;
        smf_mutex_t         mutex_pmsgq;

        if( !ppHPMsg || !(*ppHPMsg) )
        {
            rval = SMF_ERR_INVALID_PARAM;
            break;
        }

        pDev = STRUCTURE_POINTER(pmsgq_dev_t, (*ppHPMsg), hPMsg);

        smf_mutex_lock(&pDev->mutex_pmsgq);

        *ppHPMsg = 0;
        mutex_pmsgq = pDev->mutex_pmsgq;

        if( pDev->ppNode_list )
            free(pDev->ppNode_list);

        free(pDev);

        smf_mutex_unlock(&mutex_pmsgq);
        smf_mutex_destroy(&mutex_pmsgq);

    } while(0);

    return rval;
}


smf_err_t
SmfPMsgq_Node_Push(
    smf_pmsgq_handle_t      *pHPMsg,
    void                    *pNode)
{
    smf_err_t       rval = SMF_ERR_OK;
    pmsgq_dev_t     *pDev = STRUCTURE_POINTER(pmsgq_dev_t, pHPMsg, hPMsg);

    _smf_verify_handle(pHPMsg, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pNode, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->mutex_pmsgq);

    do {
        long     idx = 0l;

        if( pDev->node_cnt >= pDev->max_nodes )
        {
            log(LOG_PMSGQ, "queue full %ld/%ld\n", pDev->node_cnt, pDev->max_nodes);
            rval = SMF_ERR_QUEUE_FULL;
            break;
        }

        idx = pDev->node_cnt++;
        pDev->ppNode_list[idx] = pNode;

        _bubble_up(pDev, idx);

        pDev->hPMsg.remain_num = pDev->node_cnt - 1;

    } while(0);

    smf_mutex_unlock(&pDev->mutex_pmsgq);

    return rval;
}


smf_err_t
SmfPMsgq_Node_Pop(
    smf_pmsgq_handle_t  *pHPMsg,
    void                **ppNode)
{
    smf_err_t       rval = SMF_ERR_OK;
    pmsgq_dev_t     *pDev = STRUCTURE_POINTER(pmsgq_dev_t, pHPMsg, hPMsg);

    _smf_verify_handle(pHPMsg, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(ppNode, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->mutex_pmsgq);

    do {
        *ppNode = NULL;

        if( pDev->node_cnt == 1 )
        {
            log(LOG_PMSGQ, "%s", "queue is empty \n");
            rval = SMF_ERR_QUEUE_EMPTY;
            break;
        }

        *ppNode = pDev->ppNode_list[1];

        pDev->ppNode_list[1] = pDev->ppNode_list[--pDev->node_cnt];
        _percolate_down(pDev, 1l);

        pDev->hPMsg.remain_num = pDev->node_cnt - 1;

    } while(0);

    smf_mutex_unlock(&pDev->mutex_pmsgq);

    return rval;
}


smf_err_t
SmfPMsgq_Node_Change_Priority(
    smf_pmsgq_handle_t       *pHPMsg,
    smf_pmsgq_priority_t     *pNew_pri,
    void                     *pNode)
{
    smf_err_t       rval = SMF_ERR_OK;
    pmsgq_dev_t     *pDev = STRUCTURE_POINTER(pmsgq_dev_t, pHPMsg, hPMsg);

    _smf_verify_handle(pHPMsg, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pNew_pri, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pNode, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->mutex_pmsgq);

    do {
        long                     cur_idx = 0l;
        smf_pmsgq_priority_t     cur_pri = {{0}};

        cur_pri = *(pDev->cbPriGet(pNode));

        pDev->cbPriSet(pNode, pNew_pri);
        cur_idx = pDev->cbPosGet(pNode);

        if( pDev->cbPriCmp(&cur_pri, pNew_pri) )
            _bubble_up(pDev, cur_idx);
        else
            _percolate_down(pDev, cur_idx);

    } while(0);

    smf_mutex_unlock(&pDev->mutex_pmsgq);

    return rval;
}


smf_err_t
SmfPMsgq_Node_Peek(
    smf_pmsgq_handle_t  *pHPMsg,
    void                **ppNode)
{
    smf_err_t       rval = SMF_ERR_OK;
    pmsgq_dev_t     *pDev = STRUCTURE_POINTER(pmsgq_dev_t, pHPMsg, hPMsg);

    _smf_verify_handle(pHPMsg, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(ppNode, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->mutex_pmsgq);

    do {
        *ppNode = NULL;

        if( pDev->node_cnt == 1 )
        {
            log(LOG_PMSGQ, "%s", "queue is empty \n");
            rval = SMF_ERR_QUEUE_EMPTY;
            break;
        }

        *ppNode = pDev->ppNode_list[1];

    } while(0);

    smf_mutex_unlock(&pDev->mutex_pmsgq);

    return rval;
}


smf_err_t
SmfPMsgq_Node_Remove(
    smf_pmsgq_handle_t  *pHPMsg,
    void                *pNode)
{
    smf_err_t      rval = SMF_ERR_OK;
    pmsgq_dev_t    *pDev = STRUCTURE_POINTER(pmsgq_dev_t, pHPMsg, hPMsg);

    _smf_verify_handle(pHPMsg, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pNode, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->mutex_pmsgq);

    do {
        long         cur_idx = 0l;

        cur_idx = (long)pDev->cbPosGet(pNode);
        pDev->ppNode_list[cur_idx] = pDev->ppNode_list[--pDev->node_cnt];

        if( pDev->cbPriCmp(pDev->cbPriGet(pNode), pDev->cbPriGet(pDev->ppNode_list[cur_idx])))
            _bubble_up(pDev, cur_idx);
        else
            _percolate_down(pDev, cur_idx);

    } while(0);

    smf_mutex_unlock(&pDev->mutex_pmsgq);

    return rval;
}


smf_err_t
SmfPMsgq_Print_All(
    smf_pmsgq_handle_t  *pHPMsg,
    void                *pOutDevice,
    void                *pExtra,
    CB_PRINT_ENTRY      cbPrint)
{
    smf_err_t       rval = SMF_ERR_OK;
    pmsgq_dev_t     *pDev = STRUCTURE_POINTER(pmsgq_dev_t, pHPMsg, hPMsg);

    _smf_verify_handle(pHPMsg, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(cbPrint, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->mutex_pmsgq);

    do {
        pmsgq_dev_t         dup_dev = {{0}};
        void                **ppNode_list = 0, *pNode = 0;

        if( pDev->node_cnt == 1 )
        {
            log(LOG_PMSGQ, "%s", "queue is empty \n");
            break;
        }

        dup_dev.max_nodes  = pDev->max_nodes;
        dup_dev.node_cnt   = pDev->node_cnt;
        dup_dev.cbPriGet   = pDev->cbPriGet;
        dup_dev.cbPriCmp   = pDev->cbPriCmp;
        dup_dev.cbPosGet   = pDev->cbPosGet;

        dup_dev.cbPriSet   = _print_set_pri;
        dup_dev.cbPosSet   = _print_set_pos;


        if( !(ppNode_list = malloc(sizeof(void*) * (dup_dev.node_cnt + 1))) )
        {
            log_err("malloc fail (size= %ld)\n", sizeof(void*) * (dup_dev.node_cnt + 1));
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }

        dup_dev.ppNode_list = ppNode_list;

        memcpy(ppNode_list, pDev->ppNode_list, sizeof(void*) * dup_dev.node_cnt);

        while( dup_dev.node_cnt > 1 &&
               (pNode = ppNode_list[1]) )
        {
            ppNode_list[1] = ppNode_list[--dup_dev.node_cnt];

            _percolate_down(&dup_dev, 1l);

            cbPrint(pOutDevice, pNode, pExtra);
        }

        free(ppNode_list);
    } while(0);

    smf_mutex_unlock(&pDev->mutex_pmsgq);
    return rval;
}

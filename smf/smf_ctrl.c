/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smf_ctrl.c
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
#include "smf_ctrl.h"
#include "smf_elem_desc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
struct smf_args;
typedef smf_err_t (*CB_EXECUTE)(smf_elem_desc_t *pElem_desc, struct smf_args *pArgs, struct smf_args *pShare2Next);

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  smf device
 */
typedef struct smf_dev
{
    smf_handle_t            hSmf;
    smf_mutex_t             smf_mutex;

    smf_elem_desc_t         *pElem_head;
    smf_elem_desc_t         *pElem_cur;

} smf_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static smf_elem_desc_t      *g1stElemDesc = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
smf_elem_desc_t             g_elem_customization_desc =
{
    .elem_priv =
    {
        .uid = SMF_ELEM_CUSTOMIZATION,
        .name     = "customization",
    },
};

#include "smf_register.h"

static smf_elem_desc_t*
_find_elem_desc(
    smf_elem_id_t       elem_id,
    smf_elem_desc_t     *pFirstDesc)
{
    smf_elem_desc_t 	*pDesc = pFirstDesc;
    while( pDesc )
    {
        if( pDesc->elem_priv.uid == elem_id )
            return pDesc;
        pDesc = pDesc->next;
    }
    return 0;
}


static smf_err_t
_smf_element_init(
    smf_elem_desc_t     *pElem_desc,
    smf_args_t          *pArgs,
    smf_args_t          *pShare2Next)
{
    smf_err_t               rval = SMF_ERR_OK;
    smf_elem_priv_info_t    *pelem_priv = &pElem_desc->elem_priv;

    if( pelem_priv->cbInit )
    {
        if( (rval = pelem_priv->cbInit(pelem_priv)) )
        {
            log_err("element (x%lx, name= %s) init fail (err= x%x)\n",
                    pelem_priv->uid,
                    (pelem_priv->name) ? pelem_priv->name : "N/A",
                    rval);
        }
    }
    return rval;
}


static smf_err_t
_smf_element_deinit(
    smf_elem_desc_t     *pElem_desc,
    smf_args_t          *pArgs,
    smf_args_t          *pShare2Next)
{
    smf_err_t               rval = SMF_ERR_OK;
    smf_elem_priv_info_t    *pelem_priv = &pElem_desc->elem_priv;

    if( pelem_priv->cbDeInit )
    {
        if( (rval = pelem_priv->cbDeInit(pelem_priv)) )
        {
            log_err("element (x%lx, name= %s) init fail (err= x%x)\n",
                    pelem_priv->uid,
                    (pelem_priv->name) ? pelem_priv->name : "N/A",
                    rval);
        }
    }
    return rval;
}

static smf_err_t
_smf_element_send_msg(
    smf_elem_desc_t     *pElem_desc,
    smf_args_t          *pArgs,
    smf_args_t          *pShare2Next)
{
    smf_err_t               rval = SMF_ERR_OK;
    smf_elem_priv_info_t    *pelem_priv = &pElem_desc->elem_priv;

    if( pelem_priv->cbRecvMsg )
    {
        if( (rval = pelem_priv->cbRecvMsg(pelem_priv, pArgs->arg[0].pAddr, pShare2Next)) )
        {
            log_err("element (x%lx, name= %s) send msg fail (err= x%x)\n",
                    pelem_priv->uid,
                    (pelem_priv->name) ? pelem_priv->name : "N/A",
                    rval);
        }
    }
    return rval;
}

static smf_err_t
_smf_execute_by_order(
    smf_elem_desc_t     *pElem_desc_cur,
    smf_elem_order_t    order,
    CB_EXECUTE          cbMethod,
    smf_args_t          *pArgs)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_args_t      share_info= {.arg[0] = {0}};

    while( pElem_desc_cur )
    {
        smf_linker_t        *pLinker_cur = 0;

        switch( order )
        {
            default:
            case SMF_ELEM_ORDER_FORWARD:
                {
                    // parent element
                    if( (rval = cbMethod(pElem_desc_cur, pArgs, &share_info)) )
                        break;

                    // child elements
                    pLinker_cur = pElem_desc_cur->pLinker_head;
                    while( pLinker_cur )
                    {
                        smf_elem_desc_t     *pElem_desc_act_child = pLinker_cur->pElem_desc;

                        if( (rval = cbMethod(pElem_desc_act_child, pArgs, &share_info)) )
                            break;

                        pLinker_cur = pLinker_cur->next;
                    }

                    if( rval )      break;

                    pElem_desc_cur = pElem_desc_cur->next;
                }
                break;

            case SMF_ELEM_ORDER_BACKWARD:
                {
                    // child elements
                    pLinker_cur = pElem_desc_cur->pLinker_cur;
                    while( pLinker_cur )
                    {
                        smf_elem_desc_t     *pElem_desc_act_child = pLinker_cur->pElem_desc;

                        if( (rval = cbMethod(pElem_desc_act_child, pArgs, &share_info)) )
                            break;

                        pLinker_cur = pLinker_cur->prev;
                    }

                    if( rval )      break;

                    // parent element
                    if( (rval = cbMethod(pElem_desc_cur, pArgs, &share_info)) )
                        break;

                    pElem_desc_cur = pElem_desc_cur->prev;
                }
                break;
        }

        if( rval )      break;
    }

    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
smf_err_t
Smf_Create(
    smf_handle_t        **ppHSmf,
    smf_init_info_t     *pInit_info)
{
    smf_err_t           rval = SMF_ERR_OK;
    smf_dev_t           *pDev = 0;

    do {
        if( !ppHSmf || !pInit_info )
        {
            log_err("input null pointer: %p, %p\n", ppHSmf, pInit_info);
            rval = SMF_ERR_INVALID_PARAM;
            break;
        }

        //--------------------------
        // malloc handle
        if( !(pDev = malloc(sizeof(smf_dev_t))) )
        {
            log_err("malloc fail (size= %d)\n", sizeof(smf_dev_t));
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pDev, 0x0, sizeof(smf_dev_t));

        //-----------------------------------
        if( smf_mutex_init(&pDev->smf_mutex, 0) )
        {
            log_err("%s", "create channel mutex Fail !\n");
            rval = SMF_ERR_UNKNOWN;
            break;
        }

        _smf_register_element(&g1stElemDesc);

        *ppHSmf = &pDev->hSmf;
    } while(0);

    if( rval )
    {
        smf_handle_t        *pHTmp = &pDev->hSmf;
        Smf_Destroy(&pHTmp);
    }
    return rval;
}

smf_err_t
Smf_Destroy(
    smf_handle_t        **ppHSmf)
{
    smf_err_t           rval = SMF_ERR_OK;

    if( ppHSmf && *ppHSmf )
    {
        smf_dev_t           *pDev = STRUCTURE_POINTER(smf_dev_t, (*ppHSmf), hSmf);
        smf_mutex_t         mutex;

        smf_mutex_lock(&pDev->smf_mutex);
        *ppHSmf = 0;

        mutex = pDev->smf_mutex;

        // TODO: release element descriptor

        // destroy dev info
        free(pDev);

        smf_mutex_unlock(&mutex);
        smf_mutex_destroy(&mutex);

    }
    return rval;
}


smf_err_t
Smf_Elem_New(
    smf_handle_t            *pHSmf,
    smf_elem_id_t           elem_type,
    smf_elem_priv_info_t    **ppElem_info)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(ppElem_info, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {
        smf_elem_desc_t     *pElem_desc_pattern = 0, *pElem_desc_new = 0;

        // find target element
        pElem_desc_pattern = _find_elem_desc(elem_type, g1stElemDesc);
        if( !pElem_desc_pattern )
        {
            log_err("wrong element type (x%x)\n", elem_type);
            rval = SMF_ERR_INVALID_PARAM;
            break;
        }

        // malloc element instance
        if( !(pElem_desc_new = malloc(sizeof(smf_elem_desc_t))) )
        {
            log_err("malloc fail (size= %d)\n", sizeof(smf_elem_desc_t));
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }
        memcpy(pElem_desc_new, pElem_desc_pattern, sizeof(smf_elem_desc_t));

        pElem_desc_new->next               = 0;
        pElem_desc_new->prev               = 0;

        *ppElem_info = &pElem_desc_new->elem_priv;
    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}

smf_err_t
Smf_Elem_Del(
    smf_handle_t            *pHSmf,
    smf_elem_priv_info_t    **ppElem_info)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(ppElem_info, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(*ppElem_info, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {

        // find target element
        // TODO: malloc element instance

    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}

smf_err_t
Smf_Elem_Bind(
    smf_handle_t            *pHSmf,
    smf_elem_priv_info_t    *pElem_cur,
    smf_elem_priv_info_t    *pElem_next)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pElem_cur, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pElem_next, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {
        smf_linker_t        *pLinker_new = 0;
        smf_elem_desc_t     *pElem_desc_cur = 0, *pElem_desc_next = 0;

        // malloc linker
        if( !(pLinker_new = malloc(sizeof(smf_linker_t))) )
        {
            log_err("malloc fail (size= %d)\n", sizeof(smf_linker_t));
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pLinker_new, 0x0, sizeof(smf_linker_t));

        pElem_desc_cur  = STRUCTURE_POINTER(smf_elem_desc_t, pElem_cur, elem_priv);
        pElem_desc_next = STRUCTURE_POINTER(smf_elem_desc_t, pElem_next, elem_priv);

        pLinker_new->pElem_desc = pElem_desc_next;

        if( pElem_desc_cur->pLinker_head )
        {
            smf_linker_t    *pLinker_cur = pElem_desc_cur->pLinker_cur;

            pLinker_new->prev = pLinker_cur;
            pLinker_cur->next = pLinker_new;

            pElem_desc_cur->pLinker_cur = pLinker_new;
        }
        else
        {
            // first child element
            pElem_desc_cur->pLinker_head = pLinker_new;
            pElem_desc_cur->pLinker_cur  = pLinker_new;
        }

    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}

smf_err_t
Smf_Elem_Add(
    smf_handle_t            *pHSmf,
    smf_elem_priv_info_t    *pElem)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pElem, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {
        smf_elem_desc_t     *pElem_desc_new = STRUCTURE_POINTER(smf_elem_desc_t, pElem, elem_priv);

        pElem_desc_new->next = 0;

        if( pDev->pElem_head )
        {
            pElem_desc_new->prev  = pDev->pElem_cur;
            pDev->pElem_cur->next = pElem_desc_new;
        }
        else
        {
            // first element
            pElem_desc_new->prev = 0;
            pDev->pElem_head     = pElem_desc_new;
        }

        pDev->pElem_cur = pElem_desc_new;

    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}


smf_err_t
Smf_Start(
    smf_handle_t        *pHSmf,
    smf_elem_order_t    order,
    void                *pExtraData)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {
        smf_elem_desc_t     *pElem_desc_cur = 0;
        smf_args_t          args = {.arg[0] = {0}};

        switch( order )
        {
            default:
            case SMF_ELEM_ORDER_FORWARD:
                pElem_desc_cur = pDev->pElem_head;
                break;

            case SMF_ELEM_ORDER_BACKWARD:
                pElem_desc_cur = pDev->pElem_cur;
                break;
        }

        rval = _smf_execute_by_order(pElem_desc_cur, order, _smf_element_init, &args);
        if( rval )      break;

    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}

smf_err_t
Smf_Stop(
    smf_handle_t        *pHSmf,
    smf_elem_order_t    order,
    void                *pExtraData)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {
        smf_elem_desc_t     *pElem_desc_cur = 0;
        smf_args_t          args = {.arg[0] = {0}};

        switch( order )
        {
            default:
            case SMF_ELEM_ORDER_FORWARD:
                pElem_desc_cur = pDev->pElem_head;
                break;

            case SMF_ELEM_ORDER_BACKWARD:
                pElem_desc_cur = pDev->pElem_cur;
                break;
        }

        rval = _smf_execute_by_order(pElem_desc_cur, order, _smf_element_deinit, &args);
        if( rval )      break;

    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}

smf_err_t
Smf_Send_Msg(
    smf_handle_t        *pHSmf,
    smf_elem_order_t    order,
    void                *pMsg)
{
    smf_err_t       rval = SMF_ERR_OK;
    smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);

    _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);
    _smf_verify_handle(pMsg, SMF_ERR_INVALID_PARAM);

    smf_mutex_lock(&pDev->smf_mutex);
    do {
        smf_elem_desc_t     *pElem_desc_cur = 0;
        smf_args_t          args = {.arg[0] = {0}};

        switch( order )
        {
            default:
            case SMF_ELEM_ORDER_FORWARD:
                pElem_desc_cur = pDev->pElem_head;
                break;

            case SMF_ELEM_ORDER_BACKWARD:
                pElem_desc_cur = pDev->pElem_cur;
                break;
        }

        args.arg[0].pAddr = pMsg;
        rval = _smf_execute_by_order(pElem_desc_cur, order, _smf_element_send_msg, &args);
        if( rval )      break;

    } while(0);

    smf_mutex_unlock(&pDev->smf_mutex);

    return rval;
}

// smf_err_t
// Smf_example(
//     smf_handle_t    *pHSmf)
// {
//     smf_err_t       rval = SMF_ERR_OK;
//     smf_dev_t       *pDev = STRUCTURE_POINTER(smf_dev_t, pHSmf, hSmf);
//
//     _smf_verify_handle(pHSmf, SMF_ERR_INVALID_PARAM);
//
//     smf_mutex_lock(&pDev->smf_mutex);
//     do {
//
//         // find target element
//
//     } while(0);
//
//     smf_mutex_unlock(&pDev->smf_mutex);
//
//     return rval;
// }

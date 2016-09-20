
#include <stdio.h>
#include "smf_elem_desc.h"

#if 1 //(CFG_ENABLE_ELEM_TEST)

#include "windows.h"

#include "log.h"
#include "smf_ctrl.h"

#include "mbox.h"
#include "pthread.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct elem_mgr
{
    pthread_t           tid;

    pthread_mutex_t     mutex;

    long                isLoop;


} elem_mgr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static long
_my_elem_cmp_priority(
    smf_pmsgq_priority_t     *pPri_a,
    smf_pmsgq_priority_t     *pPri_b)
{
    return (pPri_a->u.u32_value > pPri_b->u.u32_value);
}

static smf_pmsgq_priority_t*
_my_elem_get_priority(void  *pNode)
{
    return &((mbox_t*)pNode)->base.priority;
}

static void
_my_elem_set_priority(
    void                    *pNode,
    smf_pmsgq_priority_t    *pPri)
{
    ((mbox_t*)pNode)->base.priority = *pPri;
}


static long
_my_elem_get_position(void *pNode)
{
    return ((mbox_t*)pNode)->base.position;
}

static void
_my_elem_set_position(void *pNode, long pos)
{
    ((mbox_t*)pNode)->base.position = pos;
}

static void*
_thread_my_elem(void   *arg)
{
    smf_err_t               rval = SMF_ERR_OK;
    smf_elem_priv_info_t    *pElem_prev = (smf_elem_priv_info_t*)arg;
    elem_mgr_t              *pMgr = (elem_mgr_t*)pElem_prev->pTunnelInfo[0];
    smf_pmsgq_handle_t      *pHPMsg = (smf_pmsgq_handle_t*)pElem_prev->pTunnelInfo[1];

    while( pMgr->isLoop )
    {
        mbox_t              *pMbox = 0;
        CB_MBOX_DESTROY     cb_mbox_destroy = 0;

        // pthread_mutex_lock(&pMgr->mutex);
        do {
            rval = SmfPMsgq_Node_Pop(pHPMsg, (void*)&pMbox);
            if( rval )      break;


            if( pMbox->cb_mbox_destroy )
            {
                cb_mbox_destroy = pMbox->cb_mbox_destroy;
                dbg("'%s' call delete\n", pElem_prev->name);
                cb_mbox_destroy(&pMbox, (void*)pElem_prev);
            }

        } while(0);

        // pthread_mutex_unlock(&pMgr->mutex);

        Sleep(5);
    }

    pthread_exit(NULL);
    return 0;
}

static smf_err_t
_my_elem_init(
    smf_elem_priv_info_t    *pElem_prev)
{
    smf_err_t       rval = SMF_ERR_OK;

    dbg("enter %s\n", pElem_prev->name);
    do {
        elem_mgr_t              *pMgr = 0;
        smf_pmsgq_handle_t       *pHPMsg = 0;
        smf_pmsgq_init_info_t    init_info = {0};

        if( !(pMgr = malloc(sizeof(elem_mgr_t))) )
        {
            log_err("malloc fail size= %d\n", sizeof(elem_mgr_t));
            rval = SMF_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pMgr, 0x0, sizeof(elem_mgr_t));

        pMgr->isLoop = 1;
        pthread_mutex_init(&pMgr->mutex, NULL);


        init_info.amount_nodes = 10;
        init_info.cbPriGet     = _my_elem_get_priority;
        init_info.cbPriSet     = _my_elem_set_priority;
        init_info.cbPriCmp     = _my_elem_cmp_priority;

        init_info.cbPosGet     = _my_elem_get_position;
        init_info.cbPosSet     = _my_elem_set_position;
        SmfPMsgq_Create(&pHPMsg, &init_info);

        //---------------------
        // attach
        pElem_prev->pTunnelInfo[0] = (void*)pMgr;
        pElem_prev->pTunnelInfo[1] = (void*)pHPMsg;

        pthread_create(&pMgr->tid, NULL, _thread_my_elem, (void*)pElem_prev);

    } while(0);

    return rval;
}

static smf_err_t
_my_elem_deinit(
    smf_elem_priv_info_t    *pElem_prev)
{
    smf_err_t       rval = SMF_ERR_OK;
    dbg("enter %s\n", pElem_prev->name);
    do {
        elem_mgr_t              *pMgr = (elem_mgr_t*)pElem_prev->pTunnelInfo[0];
        smf_pmsgq_handle_t      *pHPMsg = (smf_pmsgq_handle_t*)pElem_prev->pTunnelInfo[1];
        pthread_mutex_t         mutex;

        pMgr->isLoop = 0;
        pthread_join(pMgr->tid, NULL);

        pthread_mutex_lock(&pMgr->mutex);

        mutex = pMgr->mutex;


        SmfPMsgq_Destroy(&pHPMsg);

        free(pMgr);

        pthread_mutex_unlock(&mutex);
        pthread_mutex_destroy(&mutex);

    } while(0);
    return rval;
}

static smf_err_t
_my_elem_elem_recv_msg(
    smf_elem_priv_info_t    *pElem_prev,
    void                    *pMsg,
    smf_args_t              *pShare_info)
{
    smf_err_t       rval = SMF_ERR_OK;

    dbg("enter %s\n", pElem_prev->name);
    do {
        // elem_mgr_t              *pMgr = (elem_mgr_t*)pElem_prev->pTunnelInfo[0];
        smf_pmsgq_handle_t      *pHPMsg = (smf_pmsgq_handle_t*)pElem_prev->pTunnelInfo[1];
        mbox_t                  *pMbox = (mbox_t*)pMsg;

        pMbox->ref_cnt++;
        SmfPMsgq_Node_Push(pHPMsg, pMbox);

    } while(0);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
smf_elem_desc_t         g_elem_my_elem_desc =
{
    .elem_priv =
    {
        .uid       = SMF_ELEM_TEST,
        .name      = "my element",
        .cbInit    = _my_elem_init,
        .cbDeInit  = _my_elem_deinit,
        .cbRecvMsg = _my_elem_elem_recv_msg,
    },
};
#else   // #if (CFG_ENABLE_ELEM_TEST)
smf_elem_desc_t         g_elem_test_desc = {0};
#endif  // #if (CFG_ENABLE_ELEM_TEST)

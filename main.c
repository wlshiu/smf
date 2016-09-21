#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "mbox.h"
#include "log.h"
#include "smf_ctrl.h"
#include "mleak_check.h"

#include "pthread.h"
////////////////////////////////////////////////
#define MY_ELEM_UID_A     0xaaa
#define MY_ELEM_UID_B     0xbbb
#define MY_ELEM_UID_1     0x111
#define MY_ELEM_UID_2     0x222
////////////////////////////////////////////////
LOG_FLAG_s              gLog_flags = {.BitFlag[0]=1};
pthread_mutex_t         g_mutex_msg = PTHREAD_MUTEX_INITIALIZER;
////////////////////////////////////////////////
static smf_err_t
_user_elem_init(
    smf_elem_priv_info_t    *pElem_prev)
{
    smf_err_t       rval = SMF_ERR_OK;
    dbg("enter %s\n", pElem_prev->name);
    return rval;
}

static smf_err_t
_user_elem_deinit(
    smf_elem_priv_info_t    *pElem_prev)
{
    smf_err_t       rval = SMF_ERR_OK;
    dbg("enter %s\n", pElem_prev->name);
    return rval;
}

static void
_user_after_deinit(
    smf_elem_priv_info_t    *pElem_prev)
{
    unsigned char    *pBuf = pElem_prev->pTunnel_info[0];
    dbg("check pTunnel_info[0]= %p\n", pBuf);
    if( pBuf )      free(pBuf);

    pElem_prev->pTunnel_info[0] = 0;
    return;
}

static smf_err_t
_user_elem_recv_msg(
    smf_elem_priv_info_t    *pElem_prev,
    void                    *pMsg,
    smf_args_t              *pShare_info)
{
    smf_err_t       rval = SMF_ERR_OK;
     mbox_t          *pMbox = (mbox_t*)pMsg;

    pShare_info->arg[0].u32_value++;
    dbg("enter '%s', ref= %d, msg= %p, share= %d\n", pElem_prev->name, pMbox->ref_cnt, pMsg, pShare_info->arg[0].u32_value);
    return rval;
}
//=========================================================================

//=========================================================================

//=========================================================================
static int _mbox_lock(
    mbox_t      *pMbox,
    void        *pExtra_data)
{
    pthread_mutex_t     *pMutex_msg = pMbox->pTunnel_info[1];
    dbg("from= x%lx\n", pMbox->from);
    pthread_mutex_lock(pMutex_msg);
    return 0;
}

static int _mbox_unlock(
    mbox_t      *pMbox,
    void        *pExtra_data)
{
    pthread_mutex_t     *pMutex_msg = pMbox->pTunnel_info[1];
    pthread_mutex_unlock(pMutex_msg);
    dbg("from= x%lx\n", pMbox->from);
    return 0;
}

static void
_mbox_release(
    mbox_t      **ppMbox,
    void        *pExtra_data)
{
    mbox_t      *pMbox = *ppMbox;

    if( !ppMbox || !(*ppMbox) )
        return;

    if( pMbox->cb_mbox_mutex_lock )
        pMbox->cb_mbox_mutex_lock(pMbox, pExtra_data);

    dbg("ref cnt= %ld\n", pMbox->ref_cnt);
    if( pMbox->ref_cnt == 1 )
    {
        dbg("real free data '%s'\n", (char*)pMbox->data.def.pAddr);

        if( pMbox->cb_mbox_mutex_unlock )
            pMbox->cb_mbox_mutex_unlock(pMbox, pExtra_data);

        if( pMbox->cb_mbox_mutex_deinit )
            pMbox->cb_mbox_mutex_deinit(pMbox, pExtra_data);

        *ppMbox = 0;
        // real release
        free(pMbox);
        return;
    }
    else
        pMbox->ref_cnt--;

    if( pMbox->cb_mbox_mutex_unlock )
        pMbox->cb_mbox_mutex_unlock(pMbox, pExtra_data);

    return;
}
//=========================================================================

////////////////////////////////////////////////
int main()
{
    smf_handle_t        *pHSmf = 0;
    smf_init_info_t     init_info = {0};
    int                 cnt = 0;
    pthread_mutex_t     mutex_msg = PTHREAD_MUTEX_INITIALIZER;

    Smf_Create(&pHSmf, &init_info);

    {
        smf_elem_priv_info_t    *pElem_test = 0, *pElem_cust = 0;

        // element A
        Smf_Elem_New(pHSmf, SMF_ELEM_TEST, &pElem_test);
        pElem_test->uid             = MY_ELEM_UID_A;
        pElem_test->name            = "elem_A";
        pElem_test->pTunnel_info[0] = malloc(10);
        dbg("attach pTunnel_info[0]= %p\n", pElem_test->pTunnel_info[0]);
        pElem_test->cbAfterDeInit   = _user_after_deinit;
        Smf_Elem_Add(pHSmf, pElem_test);

        // element 1
        Smf_Elem_New(pHSmf, SMF_ELEM_CUSTOMIZATION, &pElem_cust);
        pElem_cust->uid       = MY_ELEM_UID_1;
        pElem_cust->name      = "elem_1";
        pElem_cust->cbInit    = _user_elem_init;
        pElem_cust->cbDeInit  = _user_elem_deinit;
        Smf_Elem_Bind(pHSmf, pElem_test, pElem_cust);

        // element 2
        Smf_Elem_New(pHSmf, SMF_ELEM_CUSTOMIZATION, &pElem_cust);
        pElem_cust->uid       = MY_ELEM_UID_2;
        pElem_cust->name      = "elem_2";
        pElem_cust->cbInit    = _user_elem_init;
        pElem_cust->cbDeInit  = _user_elem_deinit;
        pElem_cust->cbRecvMsg = _user_elem_recv_msg;
        Smf_Elem_Bind(pHSmf, pElem_test, pElem_cust);

        // element B
        Smf_Elem_New(pHSmf, SMF_ELEM_TEST, &pElem_cust);
        pElem_cust->uid       = MY_ELEM_UID_B;
        pElem_cust->name      = "elem_B";
        // pElem_cust->cbInit    = _user_elem_init;
        // pElem_cust->cbDeInit  = _user_elem_deinit;
        // pElem_cust->cbRecvMsg = _user_elem_recv_msg;
        Smf_Elem_Add(pHSmf, pElem_cust);

        // launch service
        Smf_Start(pHSmf, SMF_ELEM_ORDER_FORWARD, 0);

        {
            mbox_t    *pMbox = malloc(sizeof(mbox_t));

            memset(pMbox, 0x0, sizeof(mbox_t));
            dbg("send mbox= %p\n", pMbox);

            pMbox->base.priority.u.u32_value = 1;
            pMbox->cb_mbox_destroy           = _mbox_release;

            pMbox->from                      = 0xf78;
            pMbox->data.def.pAddr            = (void*)"WTF";

            pMbox->cb_mbox_mutex_lock        = _mbox_lock;
            pMbox->cb_mbox_mutex_unlock      = _mbox_unlock;
            pMbox->pTunnel_info[1]           = &mutex_msg;
            Smf_Send_Msg(pHSmf, SMF_ELEM_ORDER_FORWARD, (void*)pMbox);
        }

        while(cnt++ < 5)
            Sleep(500);


        {
            unsigned long       act_uid = MY_ELEM_UID_A;
            dbg("\n----------\n delete 'x%x'\n", act_uid);
            Smf_Elem_Del(pHSmf, act_uid, 0);

            dbg("----------\n");
        }

        Smf_Stop(pHSmf, SMF_ELEM_ORDER_BACKWARD, 0);
    }

    dbg("---- destroy ------\n");
    Smf_Destroy(&pHSmf);

    mlead_dump();
    return 0;
}

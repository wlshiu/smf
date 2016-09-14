#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "mbox.h"
#include "log.h"
#include "smf_ctrl.h"

////////////////////////////////////////////////
#define MY_ELEM_UID_A     0xaaa
#define MY_ELEM_UID_B     0xbbb
#define MY_ELEM_UID_1     0x111
#define MY_ELEM_UID_2     0x222
////////////////////////////////////////////////
LOG_FLAG_s    gLog_flags = {.BitFlag[0]=1};
////////////////////////////////////////////////
static smf_err_t
_user_elem_init(
    smf_elem_priv_info_t    *pElem_prev)
{
    smf_err_t       rval = SMF_ERR_OK;
    printf("%s[%d] enter %s\n", __func__, __LINE__, pElem_prev->name);
    return rval;
}

static smf_err_t
_user_elem_deinit(
    smf_elem_priv_info_t    *pElem_prev)
{
    smf_err_t       rval = SMF_ERR_OK;
    printf("%s[%d] enter %s\n", __func__, __LINE__, pElem_prev->name);
    return rval;
}

static smf_err_t
_user_elem_recv_msg(
    smf_elem_priv_info_t    *pElem_prev,
    void                    *pMsg,
    smf_args_t              *pShare_info)
{
    smf_err_t       rval = SMF_ERR_OK;
    mbox_t          *pMbox = (mbox_t*)pMsg;

    pMbox->ref_cnt++;

    pShare_info->arg[0].u32_value++;
    printf("%s[%d] enter '%s', msg= %p, %d\n", __func__, __LINE__, pElem_prev->name, pMsg, pShare_info->arg[0].u32_value);
    return rval;
}
//=========================================================================

//=========================================================================

//=========================================================================
static void
_mbox_release(
    mbox_t      *pMbox,
    void        *pExtra_data)
{
    printf("%s: ref cnt= %ld\n", __func__, pMbox->ref_cnt);
    if( pMbox->ref_cnt == 1 )
    {
        printf("%s: %s\n", __func__, (char*)pMbox->data.def.pAddr);
        // real release
        free(pMbox);
    }
    else
        pMbox->ref_cnt--;

    return;
}
//=========================================================================

////////////////////////////////////////////////
int main()
{
    smf_handle_t        *pHSmf = 0;
    smf_init_info_t     init_info = {0};
    int                 cnt = 0;

    Smf_Create(&pHSmf, &init_info);

    {
        smf_elem_priv_info_t    *pElem_test = 0, *pElem_cust = 0;

        // element A
        Smf_Elem_New(pHSmf, SMF_ELEM_TEST, &pElem_test);
        pElem_test->uid  = MY_ELEM_UID_A;
        pElem_test->name = "elem_A";
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
        Smf_Elem_New(pHSmf, SMF_ELEM_CUSTOMIZATION, &pElem_cust);
        pElem_cust->uid       = MY_ELEM_UID_B;
        pElem_cust->name      = "elem_B";
        pElem_cust->cbInit    = _user_elem_init;
        pElem_cust->cbDeInit  = _user_elem_deinit;
        pElem_cust->cbRecvMsg = _user_elem_recv_msg;
        Smf_Elem_Add(pHSmf, pElem_cust);

        // launch service
        Smf_Start(pHSmf, SMF_ELEM_ORDER_FORWARD, 0);

        {
            mbox_t    *pMbox = malloc(sizeof(mbox_t));

            memset(pMbox, 0x0, sizeof(mbox_t));

            pMbox->base.priority.u.u32_value = 1;
            pMbox->cbMboxDestroy             = _mbox_release;
            pMbox->frome                     = 0xf78;
            pMbox->data.def.pAddr            = (void*)"WTF";
            Smf_Send_Msg(pHSmf, SMF_ELEM_ORDER_FORWARD, (void*)pMbox);

        }

        while(cnt++ < 5)
            Sleep(500);


        Smf_Stop(pHSmf, SMF_ELEM_ORDER_BACKWARD, 0);
    }

    Smf_Destroy(&pHSmf);

    return 0;
}

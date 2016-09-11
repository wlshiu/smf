#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "smf_pmsgq.h"

LOG_FLAG_s    gLog_flags = {.BitFlag[0]=1};


typedef struct node
{
    smf_pmsgq_base_t    base;
    int                 val;

} node_t;



static long
_cmp_priority(
    smf_pmsgq_priority_t     *pPri_a,
    smf_pmsgq_priority_t     *pPri_b)
{
    return (pPri_a->u.u32_value > pPri_b->u.u32_value);
}

static smf_pmsgq_priority_t*
_get_priority(void  *pNode)
{
    return &((node_t*)pNode)->base.priority;
}

static void
_set_priority(
    void                    *pNode,
    smf_pmsgq_priority_t     *pPri)
{
    ((node_t*)pNode)->base.priority = *pPri;
}


static long
_get_position(void *pNode)
{
    return ((node_t*)pNode)->base.position;
}

static void
_set_position(void *pNode, long pos)
{
    ((node_t*)pNode)->base.position = pos;
}

static void
_log_node(void  *pOut_dev, void  *pNode, void *pExtra)
{
    node_t  **ppTarget_node = (node_t**)pExtra;
    node_t  *pCur_node = (node_t*)pNode;

    if( ppTarget_node && pCur_node->val == 2 )
        *ppTarget_node = pCur_node;

    fprintf(pOut_dev, "pri: %7d, val: %d\n",
            pCur_node->base.priority.u.u32_value, pCur_node->val);
}

int
main()
{
    int                     i;
    smf_pmsgq_handle_t      *pHPMsg = 0;
    smf_pmsgq_init_info_t   pmsg_init = {0};
    smf_pmsgq_priority_t    new_pri = {{0}};
    node_t                  test_node[10] = {{.val = 0}};
    node_t                  *pNode = 0;

    srand(123);

    pmsg_init.amount_nodes = 10;
    pmsg_init.cbPriGet     = _get_priority;
    pmsg_init.cbPriSet     = _set_priority;
    pmsg_init.cbPriCmp     = _cmp_priority;

    pmsg_init.cbPosGet     = _get_position;
    pmsg_init.cbPosSet     = _set_position;
    SmfPMsgq_Create(&pHPMsg, &pmsg_init);
    printf("---------- push data --------------\n");
    for(i = 0; i < 10; i++)
    {
        node_t      *pCur_node = &test_node[i];

        pCur_node->base.priority.u.u32_value = rand();
        pCur_node->val                       = i;

        SmfPMsgq_Node_Push(pHPMsg, (void*)pCur_node);
        printf("\tpri= %7d, valud= %d\n", pCur_node->base.priority.u.u32_value, i);
    }

    printf("--------- log data --------------\n");
    // &pNode => try to return a pNode
    SmfPMsgq_Print_All(pHPMsg, stderr, (void*)&pNode, _log_node);

    printf("--------- change priority --------------\n");
    new_pri.u.u32_value = 111;

    printf("change pri %7d -> 111, pos= %ld\n", pNode->base.priority.u.u32_value, pNode->base.position);
    SmfPMsgq_Node_Change_Priority(pHPMsg, &new_pri, (void*)pNode);


    printf("--------- pop data --------------\n");

    // TODO: how to release node ??
    while( !SmfPMsgq_Node_Pop(pHPMsg, (void**)&pNode) )
    {
        printf("  pri= %7d, val= %d\n", pNode->base.priority.u.u32_value, pNode->val);
    }

    SmfPMsgq_Destroy(&pHPMsg);
    return 0;
}

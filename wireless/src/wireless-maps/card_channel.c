/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* A successful enqueue is not a phone ACK. The stack dispatch frame may be
 * gone before completion: pass the nonreused 32-bit token as opaque context,
 * never the address of that frame or of a recyclable callback ticket.
 *
 * Request CF lifetime and the native session retain/release are handled by the
 * adapter and original receiver. This state never frees either on a timeout.
 */
#include "card_channel.h"
#include <string.h>

int v75_card_channel_init(struct v75_card_channel *c,int cluster)
{
    if(!c||!v75_card_count(cluster))return -1;
    memset(c,0,sizeof(*c));c->cluster=cluster;c->position=-1;
    return 0;
}
int v75_card_channel_session(struct v75_card_channel *c,uintptr_t session)
{
    if(!c)return -1;
    /* Called on every actual session edge, including pointer reuse. Serial is
     * intentionally NOT reset, so late callbacks can never name a new job. */
    c->session=0;c->pending=0;c->active=0;c->replay=1;
    c->position=-1;c->navigating=0;c->preference_generation=0;
    c->dispatch_done=0;c->callback_seen=0;
    if(c->epoch==UINT64_MAX){c->exhausted=1;return -3;}
    ++c->epoch;c->session=session;
    card_policy_reset(&c->policy,c->epoch,0);
    return 0;
}
int v75_card_channel_desire(struct v75_card_channel *c,int active,int position,
                           int navigating,uint64_t generation)
{
    int view,changed;
    if(!c||position<0||position>3||(active!=0&&active!=1)||
       (navigating!=0&&navigating!=1))return -1;
    view=v75_card_view(c->cluster,position,navigating);
    if(view<0||view>=v75_card_count(c->cluster))return -1;
    changed=active!=c->active||position!=c->position||navigating!=c->navigating||
            generation!=c->preference_generation;
    if(changed){
        if(c->revision==UINT64_MAX){c->exhausted=1;c->policy.ready=0;return -3;}
        ++c->revision;
    }
    if(active&&!c->active)c->replay=1;
    c->active=active;c->position=position;c->navigating=navigating;
    c->preference_generation=generation;
    card_policy_desire(&c->policy,position,view,c->revision);
    c->policy.ready=active&&c->session&&!c->exhausted;
    return 0;
}
int v75_card_channel_take(struct v75_card_channel *c,struct v75_card_dispatch *out)
{
    if(!c||!out)return -1;
    memset(out,0,sizeof(*out));
    if(c->exhausted||!c->session||!c->active||c->pending)return 0;
    if(c->replay&&!c->policy.busy){
        c->policy.applied_position=-1;c->policy.applied_view=-1;
        c->policy.need_idr=1;c->replay=0;
    }
    if(!card_policy_take(&c->policy,&c->command))return 0;
    if(c->serial==UINT32_MAX){
        card_policy_defer_unsent(&c->policy,&c->command);
        c->exhausted=1;c->policy.ready=0;return -3;
    }
    c->pending=++c->serial;c->dispatch_done=0;c->callback_seen=0;c->callback_status=0;
    out->token=c->pending;out->session=c->session;out->epoch=c->epoch;
    out->operation=c->command.operation;out->position=c->command.position;out->view=c->command.view;
    return 1;
}
static int finish(struct v75_card_channel *c)
{
    int result;
    if(!c->dispatch_done||!c->callback_seen)return 0;
    result=card_policy_completed(&c->policy,&c->command,c->callback_status);
    c->pending=0;return result;
}
int v75_card_channel_dispatched(struct v75_card_channel *c,uint32_t token,int status)
{
    if(!c||!token||token!=c->pending||c->dispatch_done)return 0;
    c->dispatch_done=1;
    if(status){c->callback_seen=1;c->callback_status=status;}
    return finish(c);
}
int v75_card_channel_completed(struct v75_card_channel *c,uint32_t token,int status)
{
    if(!c||!token||token!=c->pending||c->callback_seen)return 0;
    c->callback_seen=1;c->callback_status=status;
    return finish(c);
}

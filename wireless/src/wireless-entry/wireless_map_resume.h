/* Restore the same accepted alternate stream after a map/mirror menu edge.
 * This is not a new session grant and does not change periodic IDR cadence.
 * The caller must supply the complete existing map/authorization decision.
 */
static void v75_map_resume(Input *p,int wanted,int grant_idr)
{
    if(!wanted){
        retire(p);
        p->map_enabled=0;
        return;
    }
    if(!p->map_enabled||grant_idr){
        p->map_enabled=1;
        p->waiting_idr=1;
        request_idr(p);
        fprintf(stderr,"V75_MAP_RESUME_IDR menu_edge=1 accepted_session_required=1\n");
    }
}

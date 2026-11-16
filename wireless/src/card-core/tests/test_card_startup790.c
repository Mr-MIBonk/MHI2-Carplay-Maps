/* Invoke the production CF builders, preference reader, READY/poll and ACK
 * bodies in a fresh host process per saved state. Not a phone/MOST test. */
#define CARD_TRANSPORT_HELPERS_ONLY 1
#include "test_card_transport790.c"

static void write_late_preference(const char *root,int position){
 char path[512],body[128];FILE *file;int n;
 snprintf(path,sizeof(path),"%s/navigation-card.state",root);
 n=snprintf(body,sizeof(body),"format=1\nposition=%d\ngeneration=42\n",position);
 file=fopen(path,"wb");check(file!=NULL);
 check(fwrite(body,1,(size_t)n,file)==(size_t)n);
 check(fprintf(file,"crc32=%x\n",(unsigned)cpmap_card_crc((const unsigned char *)body,(size_t)n))>0);
 check(!fclose(file)); /* do not call card_refresh_preference: real poll must read it */
}

int main(int argc,char **argv){
 int wanted,late,initial,n,i;const char *root;Obj *display,*session,*second;
 check(argc==4);root=argv[1];wanted=atoi(argv[2]);late=!strcmp(argv[3],"late");
 check(wanted>=0&&wanted<=3);check(!setenv("CPMAP_CARD_STATE_DIR",root,1));
 check(!setenv("CARPLAY_ALTSCREEN_LOG","/dev/null",1));
 check(!setenv("CARPLAY_NAVIGATION_STATE_PATH","/nonexistent-cpmap-host-test/navigation.pps",1));
 initial=late?CPMAP_CARD_RIGHT:wanted;
 display=create_alt_display();check(display!=NULL);
 check(!strcmp(string(display,"initialURL"),cpmap_card_url_for(initial)));
 check(number(display,"widthPixels")==1010&&number(display,"heightPixels")==376);
 check(number(display,"initialViewArea")==cpmap_card_view_for(initial,0));drop(display);
 check(card_snapshot().position==initial);
 if(!strcmp(argv[3],"previous"))check(card_snapshot().source==2);
 session=new_obj(DICT);g_viewarea_worker_started=1;
 remember_viewarea_session(session);card_runtime_poll();check(!pending&&wire_sends==0);
 card_signal_ready(session);drain(initial,cpmap_card_view_for(initial,0));
 check(scene_sends==1&&view_sends==1&&idr_sends==1);
 if(late){
  write_late_preference(root,wanted);usleep(550000);
  drain(wanted,cpmap_card_view_for(wanted,0));
  check(card_snapshot().generation==42&&card_snapshot().source==1);
 }
 n=wire_sends;for(i=0;i<30;i++)card_runtime_poll();check(wire_sends==n&&!pending);
 /* A reconnect must reapply the saved scene without another UI selection. */
 forget_viewarea_session(session);drop(session);
 display=create_alt_display();check(display!=NULL);
 check(!strcmp(string(display,"initialURL"),cpmap_card_url_for(wanted)));drop(display);
 second=new_obj(DICT);remember_viewarea_session(second);card_runtime_poll();check(!pending);
 card_signal_ready(second);drain(wanted,cpmap_card_view_for(wanted,0));
 check(wire_sends==n+3);forget_viewarea_session(second);drop(second);check(live==0);
 printf("CARD_STARTUP_TRANSPORT_PASS position=%d mode=%s fresh_process=1 ready_gate=1 reconnect=1 no_ui_reselection=1 cf_leaks=0 checks=%d\n",wanted,argv[3],checks);
 return 0;
}

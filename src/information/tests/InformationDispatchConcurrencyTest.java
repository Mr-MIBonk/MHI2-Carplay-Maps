import de.vw.mib.asl.framework.internal.framework.ServiceManager;
import de.vw.mib.genericevents.ThreadSwitchingTarget;
import org.omonob.cpmap.information.InformationAslDispatch;
import com.mst2.carplay.navi.Mst2CarPlayNaviSinkBridge;
import com.mst2.carplay.bridge.Mst2NowPlayingBridge;
import de.vw.mib.asl.internal.carplay.target.HsmTarget;
import java.util.ArrayDeque;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

/** Host scheduling test of the production dispatcher, not a native DSI test. */
public final class InformationDispatchConcurrencyTest {
 static void check(boolean value){if(!value)throw new AssertionError();}
 static final class Target implements ThreadSwitchingTarget,Runnable {
  final ArrayDeque<Runnable> jobs=new ArrayDeque<Runnable>();
  final AtomicInteger failBudget=new AtomicInteger(8),failed=new AtomicInteger();
  final Thread owner=new Thread(this,"fixture-ASL-owner");
  boolean stopping;int active,maxQueued,completed;Throwable error;
  Target(){owner.setDaemon(true);}
  public boolean isThreadChangeNecessary(){return Thread.currentThread()!=owner;}
  public void enqueue(Runnable job){
   if(failBudget.getAndDecrement()>0){failed.incrementAndGet();throw new IllegalStateException("injected enqueue failure");}
   synchronized(this){
    jobs.addLast(job);maxQueued=Math.max(maxQueued,jobs.size());
    if(maxQueued>5)throw new AssertionError("production queue exceeded five coalesced slots");
    notifyAll();
   }
  }
  public void run(){
   try{
    while(true){
     Runnable job;
     synchronized(this){while(jobs.isEmpty()&&!stopping)wait();if(stopping)return;job=jobs.removeFirst();active++;}
     try{job.run();}finally{synchronized(this){active--;completed++;notifyAll();}}
    }
   }catch(Throwable problem){synchronized(this){error=problem;notifyAll();}}
  }
  synchronized void idle()throws Exception{
   long end=System.currentTimeMillis()+10000;
   while((!jobs.isEmpty()||active!=0)&&error==null&&System.currentTimeMillis()<end)wait(50);
   if(error!=null)throw new AssertionError(error);
   check(jobs.isEmpty()&&active==0);
  }
  void close()throws Exception{ synchronized(this){stopping=true;notifyAll();}owner.join(2000);check(!owner.isAlive()); }
 }
 static void flood(final int workers,final int iterations,final HsmTarget first,final HsmTarget second)throws Exception{
  final CountDownLatch start=new CountDownLatch(1);
  final Throwable[] errors=new Throwable[workers];Thread[] threads=new Thread[workers];
  for(int n=0;n<workers;n++){
   final int worker=n;
   threads[n]=new Thread(new Runnable(){public void run(){
    try{
     start.await();
     for(int i=0;i<iterations;i++){
      check(InformationAslDispatch.defer(i%4,"worker-"+worker+"-"+i));
      if(i%31==0)InformationAslDispatch.attach((i&1)==0?first:second);
     }
    }catch(Throwable problem){errors[worker]=problem;}
   }},"fixture-producer-"+n);threads[n].start();
  }
  start.countDown();
  for(int n=0;n<workers;n++){threads[n].join(15000);check(!threads[n].isAlive());if(errors[n]!=null)throw new AssertionError(errors[n]);}
 }
 public static void main(String[] args)throws Exception{
  Target target=new Target();ServiceManager.aslThreadSwitch=target;
  HsmTarget first=new HsmTarget(),second=new HsmTarget(),finalTarget=new HsmTarget();
  // Hold the consumer initially: simultaneous producers must not flood it.
  flood(12,2000,first,second);check(target.jobs.size()<=5&&target.maxQueued<=5);
  target.owner.start();
  try{
   target.idle();
   // Then exercise queue/drain races with a real independent consumer thread.
   flood(8,5000,first,second);target.idle();
   for(int i=0;i<4;i++)check(InformationAslDispatch.defer(i,"final-sentinel"));
   target.idle();check("final-sentinel".equals(Mst2CarPlayNaviSinkBridge.context));
   int attached=0;
   for(int i=0;i<100&&attached!=1;i++){
    attached=InformationAslDispatch.attach(finalTarget);target.idle();
   }
   check(attached==1&&Mst2NowPlayingBridge.lastTarget==finalTarget);
   check(Mst2CarPlayNaviSinkBridge.calls>0&&Mst2CarPlayNaviSinkBridge.wrongThreadCalls==0);
   check(target.failed.get()==8&&target.maxQueued<=5);
   System.out.println("INFORMATION_DISPATCH_CONCURRENCY_PASS producer_calls=64000 max_queued="+target.maxQueued+
    " completed="+target.completed+" enqueue_failures="+target.failed.get()+
    " final_target_actual_attach=1 owning_thread=1 real_native_dsi=0");
  }finally{target.close();}
 }
}

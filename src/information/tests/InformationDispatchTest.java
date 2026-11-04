import de.vw.mib.asl.framework.internal.framework.ServiceManager;
import de.vw.mib.genericevents.ThreadSwitchingTarget;
import org.omonob.cpmap.information.InformationAslDispatch;
import com.mst2.carplay.navi.Mst2CarPlayNaviSinkBridge;
import com.mst2.carplay.bridge.Mst2NowPlayingBridge;
import de.vw.mib.asl.internal.carplay.target.HsmTarget;
import java.util.*;
public final class InformationDispatchTest {
 static class Target implements ThreadSwitchingTarget {
  boolean change=true,fail;List<Runnable> jobs=new ArrayList<Runnable>();
  public boolean isThreadChangeNecessary(){return change;}
  public void enqueue(Runnable job){if(fail)throw new IllegalStateException("fixture");jobs.add(job);}
 }
 static void check(boolean b){if(!b)throw new AssertionError();}
 public static void main(String[] a) {
  check(InformationAslDispatch.defer(2,null));Target t=new Target();ServiceManager.aslThreadSwitch=t;
  for(int i=0;i<1000;i++)check(InformationAslDispatch.defer(2,null));check(t.jobs.size()==1);
  InformationAslDispatch.defer(0,null);InformationAslDispatch.defer(1,null);
  InformationAslDispatch.defer(3,"old");InformationAslDispatch.defer(3,"latest");check(t.jobs.size()==4);
  check(Mst2CarPlayNaviSinkBridge.calls==0);t.change=false;
  for(Runnable job:t.jobs)job.run();t.jobs.clear();
  check(Mst2CarPlayNaviSinkBridge.calls==4&&"latest".equals(Mst2CarPlayNaviSinkBridge.context));
  check(!InformationAslDispatch.defer(2,null));t.change=true;t.fail=true;
  check(InformationAslDispatch.defer(2,null));check(t.jobs.size()==0);t.fail=false;
  check(InformationAslDispatch.defer(2,null));check(t.jobs.size()==1);
  t.change=false;t.jobs.get(0).run();t.change=true;
  t.jobs.clear();
  HsmTarget first=new HsmTarget(),second=new HsmTarget();
  check(InformationAslDispatch.attach(null)==0);
  for(int i=0;i<1000;i++)check(InformationAslDispatch.attach(first)==0);
  check(t.jobs.size()==1&&Mst2NowPlayingBridge.calls==0);
  check(InformationAslDispatch.attach(second)==0&&t.jobs.size()==1);
  t.change=false;t.jobs.get(0).run();t.jobs.clear();t.change=true;
  check(Mst2NowPlayingBridge.calls==1&&Mst2NowPlayingBridge.lastTarget==first);
  check(InformationAslDispatch.attach(first)==1); // actual result, not enqueue
  check(InformationAslDispatch.attach(first)==0&&t.jobs.size()==1);
  t.change=false;t.jobs.get(0).run();t.jobs.clear();t.change=true;
  check(InformationAslDispatch.attach(second)==0&&t.jobs.size()==1); // no stale success
  Mst2NowPlayingBridge.success=false;
  t.change=false;t.jobs.get(0).run();t.jobs.clear();t.change=true;
  check(InformationAslDispatch.attach(second)==0&&t.jobs.size()==0);
  t.fail=true;check(InformationAslDispatch.attach(second)==0&&t.jobs.size()==0);t.fail=false;
  check(InformationAslDispatch.attach(second)==0&&t.jobs.size()==1);
  for(int i=0;i<4;i++)InformationAslDispatch.defer(i,"fixture");
  check(t.jobs.size()==5);
  System.out.println("INFORMATION_ASL_DISPATCH_HOST_PASS max_queued_jobs=5 coalescing=1 owning_thread=1 enqueue_failure_retry=1 attach_actual_result=1 target_identity=1 real_vehicle=0");
 }
}

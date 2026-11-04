package com.mst2.carplay.bridge;
import de.vw.mib.asl.internal.carplay.target.HsmTarget;
import org.omonob.cpmap.information.InformationAslDispatch;
public final class Mst2NowPlayingBridge {
 public static int calls;
 public static boolean success=true;
 public static HsmTarget lastTarget;
 private static boolean tryAttach(HsmTarget target) {
  if(InformationAslDispatch.attach(target)!=-1)throw new AssertionError("not owning thread");
  calls++;lastTarget=target;return success;
 }
}

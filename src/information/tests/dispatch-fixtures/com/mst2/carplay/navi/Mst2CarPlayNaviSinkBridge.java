package com.mst2.carplay.navi;
public final class Mst2CarPlayNaviSinkBridge {
 public static int calls,wrongThreadCalls;public static String context;
 private static void owningThread(){
  if(de.vw.mib.asl.framework.internal.framework.ServiceManager.aslThreadSwitch.isThreadChangeNecessary()){
   wrongThreadCalls++;throw new AssertionError("not owning ASL thread");
  }
 }
 private static void sendInitVolley(){owningThread();calls++;}
 private static void sendCleanupVolley(){owningThread();calls++;}
 private static void sendVcFrame(){owningThread();calls++;}
 private static void triggerBapShadows(String reason){owningThread();calls++;context=reason;}
}

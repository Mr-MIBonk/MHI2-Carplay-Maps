package org.omonob.cpmap.ui;
import java.io.*;
/** Explicit offline diagnostic entry, never run by HMI/menu activation. */
public final class OfflineRuntimeAssets{
 private OfflineRuntimeAssets(){}
 public static void main(String[] args)throws Exception{
  if(!"1".equals(System.getProperty("cpvc.offline")))throw new SecurityException("Offline diagnostic opt-in required");
  if(args.length<1)throw new IllegalArgumentException("Command required");
  if("extract".equals(args[0])){
   File directory=new File("/ramdisk/cpvc-functions");if(!directory.isDirectory()&&!directory.mkdirs())throw new IOException("RAM directory missing");
   String[] names={"cpvc-mirror-canvas-790","cpvc-mirror-canvas-791","replay-yuv-fixture","canvas-command","feature-video-owner","controls.jar"};
   for(int i=0;i<names.length;i++){
    File target=new File(directory,names[i]);if(target.exists())throw new IOException(new StringBuffer("Refusing overwrite: ").append(names[i]).toString());
    InputStream in=ClassLoader.getSystemResourceAsStream(new StringBuffer("cpvc-offline/").append(names[i]).toString());if(in==null)throw new IOException("Missing embedded asset");
    FileOutputStream out=new FileOutputStream(target);try{byte[] bytes=new byte[4096];int n;while((n=in.read(bytes))!=-1)out.write(bytes,0,n);out.getFD().sync();}finally{in.close();out.close();}
    System.out.println(new StringBuffer("OFFLINE_ASSET ").append(names[i]).append(" bytes=").append(target.length()).toString());
   }return;
  }
  ProjectionSettings s=ProjectionSettings.systemStore();
  if("info-all".equals(args[0]))s.select(2,0);
  else if("nav-off".equals(args[0]))s.setNavigationInformation(false);
  else if("music-on".equals(args[0]))s.setMusicInformation(true);
  else if("music-off".equals(args[0]))s.setMusicInformation(false);
  else if("map-off".equals(args[0])){s.select(2,0);s.setCarplayMap(false);}
  else if("mirror-on".equals(args[0]))s.setCarplayMirror(true);
  else if("mirror-off".equals(args[0]))s.setCarplayMirror(false);
  else if("map-on".equals(args[0]))s.setCarplayMap(true);
  else if("key-up".equals(args[0]) || "key-down".equals(args[0])){
   ProjectionRuntimePolicy.refreshNow();
   Class type=Class.forName("de.vw.mib.carplay.clustercontrol.Mhi2ClusterControlBridge");
   java.lang.reflect.Method method=type.getMethod("onKey2",new Class[]{Integer.TYPE,Integer.TYPE,Integer.TYPE,Integer.TYPE,Integer.TYPE});
   Integer key=new Integer("key-up".equals(args[0])?38:39);
   method.invoke(null,new Object[]{new Integer(4),key,new Integer(1),new Integer(0),new Integer(1)});
   Thread.sleep(60);
   method.invoke(null,new Object[]{new Integer(4),key,new Integer(0),new Integer(0),new Integer(1)});Thread.sleep(300);
   System.out.println(new StringBuffer("ORIGINAL_KEYSTATE_INJECTED ").append(args[0]).append(" port=").append(ClusterCommandRouter.port()).toString());
  }else if(!"read".equals(args[0]))throw new IllegalArgumentException("Unknown offline command");
  ProjectionSettings.State state=s.read();ProjectionRuntimePolicy.refreshNow();
  System.out.println(new StringBuffer("FEATURE_STATE map=").append(state.mapMask).append(" mirror=").append(state.mirrorMask).append(" navigation=").append(ProjectionRuntimePolicy.navigationAllowed()?1:0).append(" music=").append(ProjectionRuntimePolicy.musicAllowed()?1:0).append(" generation=").append(state.generation).toString());
 }
}

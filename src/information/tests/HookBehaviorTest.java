import java.nio.file.*;
import java.lang.reflect.*;
import org.omonob.cpmap.ui.*;
import org.dsi.ifc.global.ResourceLocator;
/** Injection behavior test against explicit service-free fixtures, not a car. */
public final class HookBehaviorTest{
 static int checks;static void check(boolean b){checks++;if(!b)throw new AssertionError("injected hook check "+checks);}
 static int count(Class<?> c,Object o,String n)throws Exception{return c.getField(n).getInt(o);}
 static final class Loader extends ClassLoader{
  final Path root;Loader(Path r){super(HookBehaviorTest.class.getClassLoader());root=r;}
  protected synchronized Class<?> loadClass(String name,boolean resolve)throws ClassNotFoundException{
   if(!Files.isRegularFile(root.resolve(name.replace('.','/')+".class")))return super.loadClass(name,resolve);
   Class<?> c=findLoadedClass(name);if(c==null)c=findClass(name);if(resolve)resolveClass(c);return c;
  }
  protected Class<?> findClass(String name)throws ClassNotFoundException{
   try{String path=name.replace('.','/')+".class";byte[] b=BuildNativeHook.legacy(Files.readAllBytes(root.resolve(path)));
    if(!name.endsWith("Mst2NowPlayingBridge"))b=BuildV53PolicyHook.patch(path,b);
    return defineClass(name,b,0,b.length);
   }catch(Exception e){throw new ClassNotFoundException(name,e);}
  }
 }
 public static void main(String[] a)throws Exception{
  System.setProperty("cpmap.ui.stateDir",a[1]);ProjectionSettings s=new ProjectionSettings(Path.of(a[1]).toFile());Loader l=new Loader(Path.of(a[0]));
  Class<?> nav=l.loadClass("com.mst2.carplay.navi.Mst2CarPlayNaviSinkBridge");Method write=nav.getMethod("shouldWriteNow"),override=nav.getMethod("isOverrideActive");
  s.setNavigationInformation(true);ProjectionRuntimePolicy.refreshNow();check((Boolean)write.invoke(null));
  s.setNavigationInformation(false);ProjectionRuntimePolicy.refreshNow();for(int i=0;i<5;i++)check(!(Boolean)write.invoke(null));
  check(count(nav,null,"clears")==1);check(!(Boolean)override.invoke(null));check(ProjectionRuntimePolicy.musicAllowed());
  s.setNavigationInformation(true);ProjectionRuntimePolicy.refreshNow();check((Boolean)write.invoke(null));check(count(nav,null,"resumes")==1);write.invoke(null);check(count(nav,null,"resumes")==1);
  Class<?> music=l.loadClass("de.vw.mib.bap.mqbab2.audiosd.functions.CurrentStationInfo");Object m=music.getConstructor().newInstance();Method hidden=music.getMethod("setStationInfoForMirrorLink");
  Class<?> bridge=l.loadClass("com.mst2.carplay.bridge.Mst2NowPlayingBridge");Field active=bridge.getField("callActive");
  for(int on=0;on<2;on++)for(int call=0;call<2;call++){
   s.setMusicInformation(on!=0);ProjectionRuntimePolicy.refreshNow();active.setBoolean(null,call!=0);check((Boolean)hidden.invoke(m)==(on==0||call==1));check(ProjectionRuntimePolicy.navigationAllowed());
  }
  Class<?> cover=l.loadClass("de.vw.mib.asl.internal.kombipictureserver.usecaces.CoverArt");Object c=cover.getConstructor().newInstance();Method tick=cover.getMethod("mst2TickPush");
  ResourceLocator cp=new ResourceLocator();cp.url="/var/app/icab/tmp/cp_cover_fixture.jpg";cover.getField("live").set(null,cp);
  s.setMusicInformation(false);ProjectionRuntimePolicy.refreshNow();for(int i=0;i<5;i++)tick.invoke(c);
  check(count(cover,c,"transmits")==1);check(cover.getField("last").get(c)==null);check(count(cover,c,"normalTicks")==0);
  s.setMusicInformation(true);ProjectionRuntimePolicy.refreshNow();tick.invoke(c);check(count(cover,c,"normalTicks")==1);
  for(String f:new String[]{"mst2LastPushedUrl","mst2LastPushedDaemonHash","cpvcCoverOnceLastKey"})check(cover.getField(f).get(null)==null);
  ResourceLocator radio=new ResourceLocator();radio.url="/radio/station.jpg";cover.getField("live").set(null,radio);s.setMusicInformation(false);ProjectionRuntimePolicy.refreshNow();tick.invoke(c);
  check(count(cover,c,"normalTicks")==2);check(count(cover,c,"transmits")==1);
  System.out.println("INJECTED_HOOK_BEHAVIOR_PASS checks="+checks+" clear_once=1 reenable_same_cover=1 non_carplay_preserved=1 service_fixtures=1 real_vehicle=0");
 }
}

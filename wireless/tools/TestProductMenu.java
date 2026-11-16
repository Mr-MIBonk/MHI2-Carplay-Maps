import java.io.File;
import java.lang.reflect.*;
import org.omonob.phone.PhoneConnectionSettings;
import org.omonob.cpmap.ui.PhoneMenuMessages;
import org.omonob.cpmap.ui.IndependentMenuViewBridge;
import org.omonob.cpmap.ui.UiMessages;

/** Product contracts only. No headunit or native graphics acceptance claim. */
public final class TestProductMenu {
    private static int checks;
    static void check(boolean ok,String label){checks++;if(!ok)throw new AssertionError(label);}
    public static void main(String[] args)throws Exception {
        Field field=IndependentMenuViewBridge.class.getDeclaredField("FIELDS");field.setAccessible(true);
        int[] fields=(int[])field.get(null);
        check(java.util.Arrays.equals(fields,new int[]{0,1,5}),"information, animation and duplicate wireless row removed from model");
        Method choice=IndependentMenuViewBridge.class.getDeclaredMethod("visibleChoice",new Class[]{Integer.TYPE,Integer.TYPE});
        choice.setAccessible(true);
        for(int role=0;role<2;role++)for(int index=0;index<2;index++){
            int legacy=((Integer)choice.invoke(null,new Object[]{new Integer(role),new Integer(index)})).intValue();
            int mask=org.omonob.cpmap.ui.ProjectionSettings.choiceMask(role,legacy);
            check(mask==(index==0?org.omonob.cpmap.ui.ProjectionSettings.CP:0),"visible choice maps to CP-only or OEM/off backend mask");
        }
        for(String locale:UiMessages.supportedLocales()){
            check(PhoneMenuMessages.title(locale).equals("Wireless CarPlay"),"single wireless title");
            check(PhoneMenuMessages.label(locale,0).equals("Wireless CarPlay"),"single wireless choice");
            for(int n=1;n<7;n++){
                boolean rejected=false;try{PhoneMenuMessages.label(locale,n);}catch(IllegalArgumentException expected){rejected=true;}
                check(rejected,"removed wireless setting unavailable");
            }
            check(UiMessages.get(locale,2).length()==0&&UiMessages.get(locale,3).length()==0,"removed localized feature labels");
        }
        PhoneConnectionSettings settings=new PhoneConnectionSettings(new File(args[0]));
        check(settings.read().requestedFps()==30,"default primary30");
        check(settings.read().allowAutomaticConnection(),"default wireless memory and connection retained");
        settings.manualDisconnect();check(!settings.read().allowAutomaticConnection(),"manual disconnect holds");
        settings.manualConnect();check(settings.read().allowAutomaticConnection(),"manual reconnect releases hold");
        settings.set(0,false);check(!settings.read().enabled(0),"wireless disabled");
        settings.set(0,true);check(settings.read().enabled(0),"wireless enabled");
        check(new PhoneConnectionSettings(new File(args[0])).read().requestedFps()==30,"reload primary30");
        for(int n=2;n<=6;n++){
            boolean rejected=false;try{settings.set(n,true);}catch(IllegalArgumentException expected){rejected=true;}
            check(rejected,"removed AA CarLife FPS60 setting rejected");
        }
        Class.forName("org.omonob.phone.PhoneConnectionRuntime");
        System.out.println("WIRELESS_MAPS_MENU_PRODUCT_PASS checks="+checks+" native_touch_not_tested=1");
    }
}

import java.lang.reflect.*;
import org.omonob.cpmap.ui.*;

public final class TestGenericMenu {
    public static void main(String[] args)throws Exception {
        Class type=IndependentMenuViewBridge.class;
        Field f=type.getDeclaredField("FIELDS");f.setAccessible(true);
        if(!java.util.Arrays.equals((int[])f.get(null),new int[]{0,1,5}))
            throw new AssertionError("Only map, mirroring and cards on non-wireless profiles");
        Method choice=type.getDeclaredMethod("visibleChoice",new Class[]{Integer.TYPE,Integer.TYPE});choice.setAccessible(true);
        int checks=1;
        for(int field=0;field<2;field++)for(int row=0;row<2;row++){
            int legacy=((Integer)choice.invoke(null,new Object[]{new Integer(field),new Integer(row)})).intValue();
            if(ProjectionSettings.choiceMask(field,legacy)!=(row==0?ProjectionSettings.CP:0))
                throw new AssertionError("Wrong map/mirror enum");
            checks++;
        }
        for(String language:UiMessages.supportedLocales()){
            if(UiMessages.get(language,2).length()!=0||UiMessages.get(language,3).length()!=0)
                throw new AssertionError("Removed localized information/CarLife label");
            checks++;
        }
        System.out.println("GENERIC_MAPS_MENU_PASS checks="+checks+" no_unsupported_wireless_action=1");
    }
}

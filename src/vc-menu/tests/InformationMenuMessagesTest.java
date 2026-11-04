import org.omonob.cpmap.ui.InformationMenuMessages;
import org.omonob.cpmap.ui.UiMessages;
public final class InformationMenuMessagesTest {
    public static void main(String[] args) {
        String[] locales=UiMessages.supportedLocales();
        if(locales.length!=31)throw new AssertionError("Locale count changed");
        for(int i=0;i<locales.length;i++) {
            String marker=InformationMenuMessages.marker(locales[i]);
            String label=InformationMenuMessages.label(locales[i]);
            if(marker.length()==0 || label.indexOf("("+marker+")")<0 || InformationMenuMessages.compactLabel(locales[i]).indexOf("("+marker+")")<0)
                throw new AssertionError("Experimental marker missing: "+locales[i]);
        }
        if(!InformationMenuMessages.label("zh_CN").equals("CarPlay 信息 (试验)"))
            throw new AssertionError("Chinese experimental information label changed");
        System.out.println("INFORMATION_EXPERIMENTAL_LABELS_PASS locales=31");
    }
}

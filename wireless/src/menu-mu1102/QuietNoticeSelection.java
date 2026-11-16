package org.omonob.phone;

import java.io.File;
import de.vw.mib.asl.internal.smartphoneintegration.common.SmartPhoneIntegrationGlobalProperyAccessor;

/** A connection notice belongs to the selected row, not to an already
 * connected device. Never grant a wireless session or change preferences here.
 */
final class QuietNoticeSelection {
    private QuietNoticeSelection(){}
    static boolean owns(int row,int type){
        try{
            PhoneConnectionSettings settings;
            String root=System.getProperty("omonob.phone.root");
            if("1".equals(System.getProperty("cpvc.offline"))&&root!=null)
                settings=new PhoneConnectionSettings(new File(root));
            else settings=PhoneConnectionSettings.systemStore();
            if(!settings.read().enabled(PhoneConnectionSettings.CARPLAY_CONNECTION))return false;
            return PhoneSelectionPolicy.isWirelessSelection(
                SmartPhoneIntegrationGlobalProperyAccessor.getInstance().getDsiDevicelistASLExtended(),row,type);
        }catch(Throwable unavailable){return false;}
    }
}

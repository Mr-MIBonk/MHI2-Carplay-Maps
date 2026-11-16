package org.omonob.phone;

import de.vw.mib.asl.internal.smartphoneintegration.common.DeviceASLExtended;
import org.dsi.ifc.smartphoneintegration.Device;

/** Native ASL indices address the full DSI extended list, not always row zero.
 * Keep wired/Android devices and their original IDs; only our wireless card
 * is allowed to authorize the independent wireless session policy.
 */
public final class PhoneSelectionPolicy {
    public static final int WIRELESS_ID=22339;
    private PhoneSelectionPolicy(){}
    public static boolean isWirelessSelection(DeviceASLExtended[] devices,int row,int modelMethod){
        return modelMethod==2&&devices!=null&&row>=0&&row<devices.length&&
            devices[row]!=null&&devices[row].getDeviceID()==WIRELESS_ID;
    }
    public static boolean isWiredCarPlaySelection(DeviceASLExtended[] devices,int row,int modelMethod){
        return modelMethod==2&&devices!=null&&row>=0&&row<devices.length&&
            devices[row]!=null&&devices[row].getDeviceID()!=WIRELESS_ID&&
            (devices[row].getConnectionMethod()&4)!=0;
    }
    public static boolean isWirelessDevice(DeviceASLExtended device){
        return device!=null&&device.getDeviceID()==WIRELESS_ID;
    }
    /** DSI bit 4 is verified CarPlay; bit 32 is CarLife-iOS. A device already
     * verified for CarPlay does not need the incompatible CarLife choice here.
     * Do not infer iPhone from its editable name, address or USB port.
     * Clone only filtered entries; the original native records stay intact.
     */
    public static Device[] verifiedAppleOnly(Device[] devices){
        if(devices==null)return null;
        Device[] result=null;
        for(int i=0;i<devices.length;i++){
            Device d=devices[i];
            if(d!=null&&(d.connectionMethod&4)!=0&&(d.connectionMethod&32)!=0){
                if(result==null){result=new Device[devices.length];System.arraycopy(devices,0,result,0,devices.length);}
                result[i]=new Device(d.deviceID,d.deviceName,d.connectionType,d.deviceAddress,d.connectionMethod&~32);
            }
        }
        return result==null?devices:result;
    }
}

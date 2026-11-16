// Project maintainer: omonob. Publication label: 2026-08-29.
package org.omonob.phone;

import java.io.*;
import java.net.*;
import java.lang.reflect.Field;
import de.vw.mib.asl.internal.smartphoneintegration.common.SmartPhoneIntegrationGlobalProperyAccessor;
import de.vw.mib.asl.internal.smartphoneintegration.common.DeviceASLExtended;
import de.vw.mib.genericevents.*;
import de.vw.mib.asl.framework.internal.framework.ServiceManager;

/** No audio decoder or driver changes. Wireless CarPlay only; no cluster information mapping. */
public final class PhoneConnectionRuntime implements Runnable,Target {
    private static boolean started;
    private static final Object LOCK=new Object();
    private static final String OFF="/mnt/persist/btwcp_off";
    private static final String LOG="/dev/shmem/omonob-phone.log";
    private static int records;
    private static boolean explicitPending,rejectedSession,acceptedSession;
    private static boolean receiverStartRequested;
    private static long manualAt;
    private static final int SELECT_EVENT=53091,DISCONNECT_EVENT=53092;
    private int target;
    private PhoneConnectionRuntime(){}
    private static File endpoint(String path){
        String root=System.getProperty("omonob.phone.root");
        if("1".equals(System.getProperty("cpvc.offline"))&&root!=null)
            return new File(new File(root,"runtime-flags"),path.substring(1).replace('/','_'));
        return new File(path);
    }
    public GenericEvents getMainObject(){return ServiceManager.eventMain;}
    public int getTargetId(){return target;}
    public void setTargetId(int id){target=id;}
    public void gotEvent(EventGeneric event){
        int id=event.getReceiverEventId();
        if(id==SELECT_EVENT){
            try{
                if(!store().read().enabled(PhoneConnectionSettings.CARPLAY_CONNECTION))return;
                DeviceASLExtended[] devices=SmartPhoneIntegrationGlobalProperyAccessor.getInstance().getDsiDevicelistASLExtended();
                int row=event.getInt(0),method=event.getInt(1);
                if(PhoneSelectionPolicy.isWirelessSelection(devices,row,method)){
                    synchronized(LOCK){manualAt=System.currentTimeMillis();}
                    explicitConnect();note("OMONOB_MANUAL_CONNECT_EVENT row="+row);
                }else if(PhoneSelectionPolicy.isWiredCarPlaySelection(devices,row,method)&&endpoint("/dev/shmem/wcp_cp_session").isFile()){
                    explicitDisconnect();control("bye");
                    note("OMONOB_WIRED_SELECTION_RELEASE_WIRELESS row="+row);
                }
            }catch(Throwable invalid){note("OMONOB_SELECTION_UNRESOLVED passed_to_original=1");}
        }else if(id==DISCONNECT_EVENT&&ownsSelectedWirelessCard()){
            explicitDisconnect();note("OMONOB_MANUAL_DISCONNECT_EVENT");
        }
    }
    private static PhoneConnectionSettings store()throws IOException{
        if("1".equals(System.getProperty("cpvc.offline"))){
            String root=System.getProperty("omonob.phone.root");
            if(root!=null)return new PhoneConnectionSettings(new File(root));
        }
        return PhoneConnectionSettings.systemStore();
    }
    static synchronized void note(String message){
        if(++records>2000)return;
        try{File file=endpoint(LOG);File parent=file.getParentFile();if(!parent.isDirectory())parent.mkdirs();FileOutputStream out=new FileOutputStream(file,true);try{out.write((message+"\n").getBytes("US-ASCII"));}finally{out.close();}}
        catch(Throwable ignored){}
    }
    public static void start(){
        synchronized(LOCK){if(started)return;started=true;}
        try{Thread worker=new Thread(new PhoneConnectionRuntime(),"omonobPhoneSettings");worker.setDaemon(true);worker.setPriority(1);worker.start();}
        catch(Throwable ignored){note("OMONOB_PHONE_START_FAILED");}
    }
    public void run(){
        try{
            for(int n=0;n<120&&de.vw.mib.hmi.internal.ServiceManagerCommon.hmiThreadInvoker==null;n++)Thread.sleep(500);
            if(de.vw.mib.hmi.internal.ServiceManagerCommon.hmiThreadInvoker==null){note("OMONOB_NOTICE_WAIT_EXPIRED");return;}
            ServiceManager.aslThreadSwitch.enqueue(new Runnable(){public void run(){
                try{
                    target=getMainObject().getEventDispatcher().registerTarget(PhoneConnectionRuntime.this,ServiceManager.TASK_HSM);
                    getMainObject().getServiceRegister().addObserver(generated.de.vw.mib.asl.internal.smartphoneintegration.ASLSmartPhoneIntegrationSetter.SELECT_DEVICE_LIST2,target,SELECT_EVENT);
                    getMainObject().getServiceRegister().addObserver(generated.de.vw.mib.asl.internal.smartphoneintegration.ASLSmartPhoneIntegrationSetter.DISCONNECT_CURRENT_DEVICE,target,DISCONNECT_EVENT);
                    note("OMONOB_PHONE_INTENT_OBSERVER_READY");
                }catch(Throwable failure){note("OMONOB_PHONE_INTENT_OBSERVER_FAILED");}
            }});
            OwnedDataNotice notice=new OwnedDataNotice(new File("/dev/shmem"));
            note("OMONOB_PHONE_READY decoder_changes=0");
            while(true){notice.tick();boolean accepted;synchronized(LOCK){accepted=acceptedSession;}TransportLease.refresh(accepted);Thread.sleep(20);}
        }catch(Throwable failure){note("OMONOB_PHONE_RUNTIME_FAILED");}
    }
    public static boolean allowAutoFront(){
        try{return store().read().allowAutomaticConnection();}catch(Throwable ignored){return false;}
    }
    private static void clusterAccepted(){
        try{if(WirelessClusterGrant.accepted())note("OMONOB_CLUSTER_SESSION_GRANTED");}
        catch(Throwable failure){note("OMONOB_CLUSTER_GRANT_FAILED instrument_blocked=1");}
    }
    private static void clusterEnded(){
        try{WirelessClusterGrant.ended();}
        catch(Throwable failure){note("OMONOB_CLUSTER_REVOKE_FAILED");}
    }
    public static void explicitConnect(){
        try{synchronized(LOCK){explicitPending=!acceptedSession||!endpoint("/dev/shmem/wcp_cp_session").exists();rejectedSession=false;manualAt=System.currentTimeMillis();}store().manualConnect();refreshAutoFront();note("OMONOB_EXPLICIT_CONNECT hold_cleared=1");}
        catch(Throwable ignored){note("OMONOB_CONNECT_MEMORY_FAILED");}
    }
    public static void explicitDisconnect(){
        try{synchronized(LOCK){explicitPending=false;rejectedSession=false;acceptedSession=false;manualAt=0;}TransportLease.clear();clusterEnded();store().manualDisconnect();refreshAutoFront();note("OMONOB_EXPLICIT_DISCONNECT automatic_entry_blocked=1");}
        catch(Throwable ignored){note("OMONOB_DISCONNECT_MEMORY_FAILED");}
    }
    private static void refreshAutoFront(){
        try{
            Class cp=Class.forName("de.vw.mib.asl.internal.wcpcp.WcpCp");
            boolean pending;synchronized(LOCK){pending=explicitPending;}
            Field field=cp.getDeclaredField("autoFrontAllowed");field.setAccessible(true);field.setBoolean(null,allowAutoFront()||pending);
        }catch(Throwable ignored){note("OMONOB_AUTO_FRONT_REFRESH_DEFERRED");}
    }
    private static void flag(File path,boolean present,boolean durable)throws IOException{
        if(!present){if(path.exists()&&!path.delete())throw new IOException("flag_remove_failed");return;}
        File parent=path.getParentFile();if(!parent.isDirectory()&&!parent.mkdirs())throw new IOException("flag_parent_failed");
        FileOutputStream out=new FileOutputStream(path);try{out.write(49);out.write(10);out.flush();if(durable)out.getFD().sync();}finally{out.close();}
    }
    private static void control(String command)throws IOException{
        DatagramSocket socket=new DatagramSocket();
        try{
            int port=7104;
            if("1".equals(System.getProperty("cpvc.offline"))){
                String value=System.getProperty("omonob.phone.controlPort");
                try{port=Integer.parseInt(value);}catch(Exception invalid){throw new IOException("isolated_test_port_required");}
                if(port<1||port>65535)throw new IOException("isolated_test_port_bounds");
            }
            byte[] raw=("WC "+command+"\n").getBytes("US-ASCII");socket.send(new DatagramPacket(raw,raw.length,InetAddress.getByName("127.0.0.1"),port));
        }
        finally{socket.close();}
    }
    private static void ensureReceiver(){
        if("1".equals(System.getProperty("cpvc.offline")))return;
        if(endpoint("/dev/shmem/wcp_receiver.pid").isFile())return;
        synchronized(LOCK){if(receiverStartRequested)return;receiverStartRequested=true;}
        try{
            Thread worker=new Thread(new Runnable(){public void run(){
                try{Runtime.getRuntime().exec(new String[]{"sh","-c","sh /mnt/ota/wcp_autostart.sh </dev/null >/dev/null 2>&1 &"});note("OMONOB_RECEIVER_START_REQUESTED");}
                catch(Throwable failure){note("OMONOB_RECEIVER_START_FAILED");}
                finally{synchronized(LOCK){receiverStartRequested=false;}}
            }},"omonobReceiverStart");worker.setDaemon(true);worker.setPriority(1);worker.start();
        }catch(Throwable failure){synchronized(LOCK){receiverStartRequested=false;}note("OMONOB_RECEIVER_START_FAILED");}
    }
    public static void preferencesChanged(PhoneConnectionSettings.State state,int field)throws IOException{
        if(field>=2&&field<=5){note("OMONOB_MENU_ONLY_PREFERENCE_SAVED backend_calls=0");return;}
        if(field==PhoneConnectionSettings.FPS60){
            flag(endpoint("/mnt/persist/omonob-phone/fps30"),!state.enabled(PhoneConnectionSettings.FPS60),true);
            note("OMONOB_FPS_PREFERENCE_SAVED effect=next_start");return;
        }
        flag(endpoint(OFF),!state.enabled(PhoneConnectionSettings.CARPLAY_CONNECTION),true);
        refreshAutoFront();
        if(!state.enabled(PhoneConnectionSettings.CARPLAY_CONNECTION)){TransportLease.clear();clusterEnded();control("bye");}
        else{
            ensureReceiver();
            if(state.allowAutomaticConnection()&&!endpoint("/dev/shmem/wcp_cp_session").exists())flag(endpoint("/dev/shmem/wcp_cp_connect_req"),true,false);
        }
        note("OMONOB_PHONE_PREFERENCES_APPLIED fps_effect=next_start");
    }
    public static void nativeConnectIntent(){
        long now=System.currentTimeMillis(),at;synchronized(LOCK){at=manualAt;}
        if(at>0&&now>=at&&now-at<=30000){explicitConnect();try{TransportLease.explicitWireless();control("wifi");note("OMONOB_EXPLICIT_WIRELESS_TRANSPORT");}catch(IOException failure){TransportLease.clear();note("OMONOB_TRANSPORT_PREFERENCE_FAILED");}}
    }
    public static boolean acceptIncomingSession(){
        try{
            PhoneConnectionSettings.State state=store().read();boolean pending,accepted;
            synchronized(LOCK){pending=explicitPending&&manualAt>0&&System.currentTimeMillis()>=manualAt&&System.currentTimeMillis()-manualAt<=120000;accepted=acceptedSession;}
            if(state.enabled(0)&&!state.manualHold()&&(state.enabled(1)||pending||accepted)){
                synchronized(LOCK){explicitPending=false;rejectedSession=false;acceptedSession=true;}
                clusterAccepted();note("OMONOB_SESSION_ACCEPTED");return true;
            }
            boolean send;synchronized(LOCK){send=!rejectedSession;rejectedSession=true;}
            clusterEnded();if(send){control("bye");note("OMONOB_SESSION_REJECTED manual_selection_required=1");}
        }catch(Throwable failure){clusterEnded();note("OMONOB_SESSION_POLICY_FAILED");}
        return false;
    }
    public static void sessionEnded(){synchronized(LOCK){rejectedSession=false;acceptedSession=false;if(!explicitPending)manualAt=0;}clusterEnded();note("OMONOB_SESSION_ENDED");}
    static boolean ownsSelectedWirelessCard(){
        try{
            if(!store().read().enabled(PhoneConnectionSettings.CARPLAY_CONNECTION))return false;
            return PhoneSelectionPolicy.isWirelessDevice(SmartPhoneIntegrationGlobalProperyAccessor.getInstance().getCurrentConnectedDevice());
        }catch(Throwable ignored){return false;}
    }
}

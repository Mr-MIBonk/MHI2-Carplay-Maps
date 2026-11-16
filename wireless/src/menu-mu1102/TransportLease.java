package org.omonob.phone;
import java.io.*;

/** RAM-only preference for an explicit native wireless-card connection.
 * Never created for an automatic connection without an attached USB phone.
 * The native reader cancels it on unplug; later USB insertion keeps priority.
 */
public final class TransportLease {
    private static final String PATH="/ramdisk/v75-wireless-transport.intent";
    private static boolean selected;
    private static long nextRefresh;
    private TransportLease(){}
    private static File file(String path){
        String root=System.getProperty("omonob.phone.root");
        if("1".equals(System.getProperty("cpvc.offline"))&&root!=null)
            return new File(new File(root,"runtime-flags"),path.substring(1).replace('/','_'));
        return new File(path);
    }
    private static void writeLease(long now)throws IOException{
        File dest=file(PATH),stage=file(PATH+".next");File parent=dest.getParentFile();
        if(!parent.isDirectory()&&!parent.mkdirs())throw new IOException("transport_parent");
        if(stage.exists()&&!stage.delete())throw new IOException("transport_stage");
        FileOutputStream out=new FileOutputStream(stage);
        try{out.write(("V75_TRANSPORT1\n"+(now+120000)+"\n").getBytes("US-ASCII"));out.flush();}finally{out.close();}
        // umask of the HMI is not assumed. The C reader accepts only root's
        // regular RAM file, exact bounded content and a short expiry.
        if(!stage.renameTo(dest)){stage.delete();throw new IOException("transport_commit");}
        nextRefresh=now+30000;
    }
    public static synchronized void explicitWireless()throws IOException{
        selected=file("/dev/ipod0").exists()||file("/dev/io-cdd/ipod0").exists();
        if(selected)writeLease(System.currentTimeMillis());else clear();
    }
    public static synchronized void refresh(boolean accepted){
        if(!selected||!accepted)return;
        if(!file(PATH).isFile()||(!file("/dev/ipod0").exists()&&!file("/dev/io-cdd/ipod0").exists())){clear();return;}
        long now=System.currentTimeMillis();if(now<nextRefresh)return;
        try{writeLease(now);}catch(IOException failure){clear();}
    }
    public static synchronized void clear(){
        selected=false;nextRefresh=0;file(PATH).delete();file(PATH+".next").delete();
    }
}

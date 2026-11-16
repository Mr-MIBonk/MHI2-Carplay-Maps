package org.omonob.phone;

import java.io.*;

/** Connection-policy grant only. Does not select a device, request foreground,
 * change a saved preference, launch a process or touch audio/network state. */
public final class WirelessClusterGrant {
    private static final String OWNER="/ramdisk/v75-wireless-native.owner";
    private static final String PREFIX="/ramdisk/.v75-native-";
    private static final Object LOCK=new Object();
    private WirelessClusterGrant(){}
    private static File path(String logical)throws IOException {
        String root=System.getProperty("v75.grant.root");
        if("1".equals(System.getProperty("cpvc.offline"))&&root!=null)
            return new File(root,logical.substring(1));
        return new File(logical);
    }
    private static String read(File file,int bound)throws IOException {
        long size=file.length();if(!file.isFile()||size<1||size>bound)throw new IOException("grant_read_bound");
        byte[] b=new byte[(int)size];FileInputStream in=new FileInputStream(file);int at=0,n;
        try{while(at<b.length&&(n=in.read(b,at,b.length-at))>0)at+=n;if(at!=b.length||in.read()!=-1)throw new IOException("grant_read_changed");}
        finally{in.close();}
        for(int i=0;i<b.length;i++)if((b[i]<32&&b[i]!=10)||b[i]>126)throw new IOException("grant_ascii");
        return new String(b,"US-ASCII");
    }
    private static String[] lines(String value,int expected)throws IOException {
        String[] fields=new String[expected];int at=0;
        for(int i=0;i<expected;i++){int end=value.indexOf('\n',at);if(end<at)throw new IOException("grant_lines");fields[i]=value.substring(at,end);at=end+1;}
        if(at!=value.length())throw new IOException("grant_trailing");return fields;
    }
    private static int number(String value)throws IOException {
        if(value.length()<1||value.length()>10||value.charAt(0)=='0')throw new IOException("grant_integer");
        for(int i=0;i<value.length();i++)if(value.charAt(i)<'0'||value.charAt(i)>'9')throw new IOException("grant_integer");
        try{int n=Integer.parseInt(value);if(n<1)throw new IOException("grant_integer");return n;}
        catch(NumberFormatException invalid){throw new IOException("grant_integer");}
    }
    private static String[] owner()throws IOException {
        String[] f=lines(read(path(OWNER),100),3);
        if(!"V75_NATIVE1".equals(f[0]))throw new IOException("grant_owner");number(f[1]);
        if(f[2].length()!=53||!f[2].startsWith(PREFIX))throw new IOException("grant_scope");
        for(int i=21;i<53;i++)if("0123456789abcdef".indexOf(f[2].charAt(i))<0)throw new IOException("grant_scope");
        return f;
    }
    public static boolean accepted()throws IOException {
        synchronized(LOCK){
            if(!path(OWNER).isFile())return false; // Wired-only installation.
            String[] o=owner();int next=1;File feed=path(o[2]+"/cluster-feed.state");
            if(feed.exists()){
                String[] f=lines(read(feed,100),4);
                if(!"V75_FEED1".equals(f[0])||!o[1].equals(f[1]))throw new IOException("grant_feed_owner");
                next=number(f[2]);
                if("0".equals(f[3])){if(next==2147483647)throw new IOException("grant_exhausted");next++;}
                else if(!"1".equals(f[3]))throw new IOException("grant_feed_state");
            }
            // Exact receiver generation AND exact next/current feed sequence.
            String value="V75_GRANT1\n"+o[1]+"\n"+next+"\n";
            File target=path(o[2]+"/cluster-session.accepted"),tmp=path(o[2]+"/cluster-session.next");
            if(!tmp.createNewFile())throw new IOException("grant_stage_exists");
            boolean committed=false;
            try{
                FileOutputStream out=new FileOutputStream(tmp);
                try{out.write(value.getBytes("US-ASCII"));out.flush();out.getFD().sync();}finally{out.close();}
                if(!read(path(OWNER),100).equals("V75_NATIVE1\n"+o[1]+"\n"+o[2]+"\n"))throw new IOException("grant_owner_changed");
                if(!tmp.renameTo(target)){
                    // Windows fixture JVMs cannot rename over a file. Missing
                    // receipt is fail-closed; QNX takes the atomic first path.
                    if(!target.isFile()||!target.delete()||!tmp.renameTo(target))throw new IOException("grant_commit");
                }
                committed=true;return true;
            }finally{if(!committed)tmp.delete();}
        }
    }
    public static void ended()throws IOException {
        synchronized(LOCK){
            if(!path(OWNER).isFile())return;
            String[] o=owner();File file=path(o[2]+"/cluster-session.accepted");
            if(file.exists()&&!file.delete())throw new IOException("grant_remove");
        }
    }
}

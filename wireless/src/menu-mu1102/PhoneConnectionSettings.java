// Project maintainer: omonob. Publication label: 2026-08-29.
package org.omonob.phone;

import java.io.*;
import java.util.Vector;
import java.util.zip.CRC32;

/** Two-slot CRC preferences. Wireless CarPlay only; primary request is fixed at 30 FPS. */
public final class PhoneConnectionSettings {
    public static final int CARPLAY_CONNECTION=0,CARPLAY_MEMORY=1,FPS60=6;
    private static final int HOLD=128,DEFAULT=3;
    private static final Object LOCK=new Object();
    private final File directory;
    public static final class State {
        public final int flags;
        public final long generation;
        public final String source;
        State(int f,long g,String s){flags=f&131;generation=g;source=s;}
        public boolean enabled(int field){check(field);return field!=FPS60&&(flags&(1<<field))!=0;}
        public boolean manualHold(){return (flags&HOLD)!=0;}
        public boolean allowAutomaticConnection(){return enabled(CARPLAY_CONNECTION)&&enabled(CARPLAY_MEMORY)&&!manualHold();}
        public int requestedFps(){return 30;}
    }
    public PhoneConnectionSettings(File root)throws IOException{
        if(root==null)throw new IOException("settings_directory_required");directory=root.getCanonicalFile();
    }
    public static PhoneConnectionSettings systemStore()throws IOException{return new PhoneConnectionSettings(new File("/mnt/persist/omonob-phone"));}
    private static void check(int field){if(field!=CARPLAY_CONNECTION&&field!=CARPLAY_MEMORY&&field!=FPS60)throw new IllegalArgumentException("unknown_phone_setting");}
    private File slot(int n){return new File(directory,"phone-settings."+n);}
    private static String[] fields(String text){
        Vector list=new Vector();int from=0,at;
        while((at=text.indexOf('|',from))>=0){list.addElement(text.substring(from,at));from=at+1;}
        list.addElement(text.substring(from));String[] out=new String[list.size()];list.copyInto(out);return out;
    }
    private static long crc(String text)throws IOException{
        CRC32 crc=new CRC32();byte[] raw=text.getBytes("US-ASCII");crc.update(raw,0,raw.length);return crc.getValue();
    }
    private State parse(File file)throws IOException{
        if(!file.exists())return null;
        if(!file.isFile()||file.length()<1||file.length()>128)return null;
        FileInputStream in=new FileInputStream(file);ByteArrayOutputStream bytes=new ByteArrayOutputStream();
        try{int b;while((b=in.read())!=-1){if(b>127||b=='\r'||bytes.size()>=128)return null;bytes.write(b);}}
        finally{in.close();}
        String text=new String(bytes.toByteArray(),"US-ASCII");
        if(!text.endsWith("\n")||text.indexOf('\n')!=text.length()-1)return null;
        String[] part=fields(text.substring(0,text.length()-1));
        if(part.length!=4||!"OMONOBPHONE1".equals(part[0]))return null;
        try{
            long generation=Long.parseLong(part[1]);int flags=Integer.parseInt(part[2]);long expected=Long.parseLong(part[3],16);
            if(generation<0||flags<0||flags>255||expected!=crc(part[0]+"|"+part[1]+"|"+part[2]))return null;
            return new State(flags,generation,"saved");
        }catch(NumberFormatException invalid){return null;}
    }
    public State read()throws IOException{synchronized(LOCK){return readLocked();}}
    private State readLocked()throws IOException{
        State a=parse(slot(0)),b=parse(slot(1));
        if(a==null&&b==null){
            if(slot(0).exists()||slot(1).exists())throw new IOException("both_phone_slots_invalid");
            return new State(DEFAULT,0,"default");
        }
        if(a==null)return b;if(b==null)return a;
        if(a.generation==b.generation&&a.flags!=b.flags)throw new IOException("phone_slots_split_brain");
        return a.generation>=b.generation?a:b;
    }
    private State commit(State before,int flags)throws IOException{
        if(flags==before.flags)return before;
        if(before.generation==Long.MAX_VALUE)throw new IOException("generation_exhausted");
        if(!directory.isDirectory()&&!directory.mkdirs())throw new IOException("preference_directory_create_failed");
        long next=before.generation+1;String body="OMONOBPHONE1|"+next+"|"+flags;
        byte[] raw=(body+"|"+Long.toHexString(crc(body))+"\n").getBytes("US-ASCII");
        File target=slot((int)(next&1)),pending=new File(directory,"phone-settings.next");
        if(pending.exists()&&!pending.delete())throw new IOException("stale_pending_remove_failed");
        FileOutputStream out=new FileOutputStream(pending);
        try{out.write(raw);out.flush();out.getFD().sync();}finally{out.close();}
        if(parse(pending)==null)throw new IOException("pending_crc_readback_failed");
        if(target.exists()&&!target.delete())throw new IOException("old_slot_remove_failed");
        if(!pending.renameTo(target))throw new IOException("slot_switch_failed");
        State result=parse(target);
        if(result==null||result.generation!=next||result.flags!=flags)throw new IOException("committed_readback_failed");
        return result;
    }
    public State set(int field,boolean on)throws IOException{
        check(field);if(field==FPS60&&on)throw new IllegalArgumentException("primary_fps_fixed_30");synchronized(LOCK){State old=readLocked();int mask=1<<field,flags=on?old.flags|mask:old.flags&~mask;
            if(field==CARPLAY_CONNECTION)flags=on?flags&~HOLD:flags|HOLD;
            return commit(old,flags);}
    }
    public State manualDisconnect()throws IOException{synchronized(LOCK){State old=readLocked();return commit(old,old.flags|HOLD);}}
    public State manualConnect()throws IOException{synchronized(LOCK){State old=readLocked();return commit(old,old.flags&~HOLD);}}
}

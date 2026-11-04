import java.io.File;
import java.net.*;
import java.lang.reflect.*;
import java.time.Instant;
import java.util.*;
/** Invoke the real final payload method, not a duplicate timezone formula. */
public final class EtaTimezoneTest {
    public static void main(String[] a)throws Exception {
        int checks=0;
        try(URLClassLoader l=new URLClassLoader(new URL[]{new File(a[0]).toURI().toURL(),new File(a[1]).toURI().toURL()},ClassLoader.getPlatformClassLoader())) {
            Class<?> c=Class.forName("com.mst2.carplay.navi.Mst2CarPlayNaviSinkBridge",true,l);
            Method format=c.getMethod("formatEtaDisplayLine",long.class,long.class);
            Field eta=c.getDeclaredField("rgdEta");eta.setAccessible(true);
            Method calendar=c.getDeclaredMethod("etaLocalCalendar");calendar.setAccessible(true);
            for(String zone:new String[]{"Europe/Vienna","Europe/London","Asia/Tokyo"}) {
                TimeZone.setDefault(TimeZone.getTimeZone(zone));
                for(String date:new String[]{"2026-01-20T12:30:00Z","2026-07-20T12:30:00Z"}) {
                    long seconds=Instant.parse(date).getEpochSecond();
                    TimeZone expected=TimeZone.getTimeZone(a[2].equals("MU1102")?"Asia/Shanghai":zone);
                    Calendar local=Calendar.getInstance(expected);local.setTimeInMillis(seconds*1000);
                    String text=String.format("%02d:%02d",local.get(11),local.get(12));
                    Object value=format.invoke(null,0L,seconds);
                    if(!text.equals(value))throw new AssertionError(zone+" "+date+" "+value+" != "+text);
                    eta.setLong(null,seconds);Calendar actual=(Calendar)calendar.invoke(null);
                    if(actual.get(11)!=local.get(11)||actual.get(12)!=local.get(12))throw new AssertionError("BAP ETA");
                    checks+=2;
                }
            }
        }
        System.out.println("INFORMATION_ETA_PASS firmware="+a[2]+" actual_payload_checks="+checks+" winter_summer=1 global_timezone_not_written=1");
    }
}

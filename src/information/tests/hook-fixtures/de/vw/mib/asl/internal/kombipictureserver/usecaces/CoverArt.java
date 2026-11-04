package de.vw.mib.asl.internal.kombipictureserver.usecaces;
import org.dsi.ifc.global.ResourceLocator;
public final class CoverArt {
 public static ResourceLocator live;
 public static String mst2LastPushedUrl="same",mst2LastPushedDaemonHash="same",cpvcCoverOnceLastKey="same";
 public int transmits,normalTicks;
 public ResourceLocator last;
 private static ResourceLocator mst2BuildLiveRL(){return live;}
 private void _transmitCoverArt(ResourceLocator value){last=value;transmits++;}
 public void mst2TickPush(){normalTicks++;}
}

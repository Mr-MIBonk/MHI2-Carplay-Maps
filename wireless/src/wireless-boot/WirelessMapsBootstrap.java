// Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29.
package org.omonob.cpmap.wireless;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.omonob.phone.PhoneConnectionRuntime;

/** Connection policy only. No metadata sink, cover worker or navigation parser. */
public final class WirelessMapsBootstrap implements BundleActivator {
    public void start(BundleContext context){
        if(!"1".equals(System.getProperty("cpvc.offline")))PhoneConnectionRuntime.start();
    }
    public void stop(BundleContext context){
        // Do not stop an active CarPlay/audio session on bundle refresh.
    }
}

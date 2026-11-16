// Maintainer: omonob. Source date: 2026-08-29.
package org.omonob.cpmap.ui;
/** Single supported wireless choice. No Android Auto, CarLife or FPS switch. */
public final class PhoneMenuMessages {
    private PhoneMenuMessages(){}
    public static String title(String language){return "Wireless CarPlay";}
    public static String label(String language,int field){
        if(field!=0)throw new IllegalArgumentException("wireless_carplay_only");
        return "Wireless CarPlay";
    }
}

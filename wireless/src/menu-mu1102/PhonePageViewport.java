package org.omonob.cpmap.ui;

import de.vw.mib.widgets.models.WidgetListModel;
import de.vw.mib.widgets.models.ViewModel;

/** Reset only our new page's inherited viewport, not its saved preferences.
 * The OEM parent may have scrolled down to expose the phone-settings entry.
 * Copying its first-item index/position must not hide this page's first row.
 */
public final class PhonePageViewport {
    private PhonePageViewport() { }

    /** A list appended after View.init() has no LUT until its own init(view).
     * validateList() alone cannot initialize it: count=0 also causes the OEM
     * renderer to skip child zero as if it were an empty template interval.
     * The caller stores this result per fresh list, never in preferences.
     */
    public static boolean initializeOnce(WidgetListModel list,ViewModel view,boolean initialized) {
        if(list==null || view==null)throw new IllegalArgumentException("owned_list_and_view_required");
        if(!initialized)list.init(view);
        return true;
    }

    public static void resetToTop(WidgetListModel list) {
        if(list==null)throw new IllegalArgumentException("phone_list_required");
        list.set_firstItemIndex(0);
        list.set_firstItemIndexPosition(0);
        list.set_selectedIndex(0);
    }
}

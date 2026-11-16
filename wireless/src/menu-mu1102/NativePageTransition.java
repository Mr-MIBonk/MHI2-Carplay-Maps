package org.omonob.cpmap.ui;

import de.vw.mib.animation.easing.EasingParams;
import de.vw.mib.widgets.models.WidgetModel;

/** Use the existing view's native animator without changing its shared gate.
 * MU1102 View.enableAnimations() decrements a nesting counter; it is not an
 * idempotent boolean setter. An unmatched enable makes the counter negative
 * and disables animations for the entire OEM view, including its dropdowns.
 * The owner View alone balances disable/enable around its normal lifecycle.
 */
public final class NativePageTransition {
    private NativePageTransition() { }

    public static void configure(WidgetModel page, WidgetModel donor) {
        EasingParams[] source=donor==null?null:donor.get_easing();
        int size=Math.max(WidgetModel.EASING_INDEX_COUNT_WIDGET,source==null?0:source.length);
        EasingParams[] easing=new EasingParams[size];
        for(int i=0;source!=null && i<source.length;i++)
            if(source[i]!=null)easing[i]=new EasingParams(source[i]);
        int[] slots={WidgetModel.EASING_INDEX_FADE_VISIBLE,WidgetModel.EASING_INDEX_FADE_INVISIBLE};
        for(int i=0;i<slots.length;i++) {
            int slot=slots[i];
            if(easing[slot]==null || easing[slot].getDuration()<=0 ||
               easing[slot].getEasingClass()==EasingParams.EASING_CLASS_NONE)
                easing[slot]=new EasingParams(EasingParams.EASING_KIND_OUT,
                    EasingParams.EASING_CLASS_CUBIC,200,0);
            easing[slot].setStartDelay(0);
        }
        page.set_easing(easing);
    }

    public static void show(WidgetModel page) {
        page.set_visible(true);
        page.repaint();
    }

    public static void hide(WidgetModel page) {
        page.set_visible(false);
    }
}

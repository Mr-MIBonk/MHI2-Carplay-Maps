package org.omonob.cpmap.ui;
/** G11 viewport bounds using the original skin's item, spacing and borders. */
public final class NativeSelectorGeometry {
    private NativeSelectorGeometry() { }
    public static int contentHeight(int count,int itemHeight,int spacing) {
        // Older Skoda G11 skins deliberately use negative spacing between
        // template items. The pitch must be positive; the gap need not be.
        if(count<1||count>8||itemHeight<1||itemHeight+spacing<1)throw new IllegalArgumentException("Invalid native list geometry");
        return count*itemHeight+(count-1)*spacing;
    }
    public static int visibleRows(int count,int available,int top,int bottom,int itemHeight,int spacing) {
        if(count<1||count>8||top<0||bottom<0||available<top+bottom+itemHeight)
            throw new IllegalArgumentException("Native viewport cannot contain one option");
        int rows=count;
        while(rows>1 && contentHeight(rows,itemHeight,spacing)+top+bottom>available)rows--;
        return rows;
    }
    /** Match OEM PushUp.calculateDimension: child y is part of its bounds. */
    public static int popupHeight(int listY,int viewportHeight,int insetTop,int insetBottom) {
        if(listY<0||viewportHeight<1||insetTop<0||insetBottom<0)
            throw new IllegalArgumentException("Invalid native popup bounds");
        return listY+viewportHeight+insetTop+insetBottom;
    }
    /** Keep the OEM anchor position unless it falls outside its content area. */
    public static int top(int nativeY,int contentY,int available,int height) {
        if(height<1||available<height)throw new IllegalArgumentException("Popup exceeds native content area");
        return Math.max(contentY,Math.min(nativeY,contentY+available-height));
    }
}

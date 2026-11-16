package org.omonob.cpmap.ui;

import de.vw.mib.view.internal.AbstractViewHandler;
import de.vw.mib.widgets.models.*;
import java.io.FileOutputStream;
import java.lang.reflect.Array;
import java.lang.reflect.Method;

/** Bounded, read-only widget inspection inside the isolated firmware image. */
public final class OemMenuInspector {
    private OemMenuInspector() { }
    public static void capture(AbstractViewHandler handler) {
        try {
            ViewModel view=handler.getView();if(view==null)return;
            StringBuffer out=new StringBuffer("VIEW ").append(view.get_name()).append('\n');
            dropDowns(view,out);
            walk(view,0,out);
            FileOutputStream file=new FileOutputStream("/tmp/cpmap-oem-view.txt");
            try{file.write(out.toString().getBytes("UTF-8"));}finally{file.close();}
        }catch(Throwable ignored) { }
    }
    private static void dropDowns(WidgetModel widget,StringBuffer out)throws Exception{
        if(widget instanceof DropDownModel)walk(widget,0,out);
        else if(widget instanceof ContainerModel){
            WidgetModel[] children=((ContainerModel)widget).getChildren();
            for(int i=0;children!=null && i<children.length;i++)dropDowns(children[i],out);
        }
    }
    private static void walk(WidgetModel widget,int depth,StringBuffer out)throws Exception {
        if(depth>20 || out.length()>180000)return;
        out.append("W ").append(depth).append(' ').append(widget.get_targetId()).append(' ').append(widget.getClass().getName())
            .append(" name=").append(widget.get_name()).append(" xywh=").append(widget.get_x()).append(',').append(widget.get_y()).append(',')
            .append(widget.get_width()).append(',').append(widget.get_height()).append(" visible=").append(widget.is_visible()).append(" layoutable=").append(widget.is_layoutable());
        if(widget.getController()!=null)out.append(" controller=").append(widget.getController().getClass().getName());
        out.append('\n');
        Method[] methods=widget.getClass().getMethods();
        for(int i=0;i<methods.length;i++) {
            Method method=methods[i];String name=method.getName();
            if(!(name.startsWith("get_") || name.startsWith("is_")) || method.getParameterTypes().length!=0)continue;
            if(name.equals("get_easing") || name.equals("get_easingStates") || name.equals("get_textColors") || name.equals("get_glowColors"))continue;
            try {out.append(" P ").append(name).append('=').append(value(method.invoke(widget,new Object[0]))).append('\n');}
            catch(Throwable ignored) { }
        }
        if(widget instanceof ContainerModel){
            WidgetModel[] children=((ContainerModel)widget).getChildren();
            for(int i=0;children!=null && i<children.length;i++)walk(children[i],depth+1,out);
        }
    }
    private static String value(Object object){
        if(object==null)return "null";
        if(object instanceof WidgetModel)return new StringBuffer("#").append(((WidgetModel)object).get_targetId()).toString();
        if(object.getClass().isArray()){
            StringBuffer list=new StringBuffer("[");int length=Math.min(Array.getLength(object),3);
            for(int i=0;i<length;i++){if(i>0)list.append(';');list.append(value(Array.get(object,i)));}
            return list.append(']').toString();
        }
        String text=object.toString().replace('\n',' ');return text.length()>260?text.substring(0,260):text;
    }
}

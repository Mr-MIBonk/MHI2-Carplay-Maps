import org.omonob.cpmap.ui.NativeSelectorGeometry;
public final class NativeSelectorGeometryTest {
    private static void check(boolean value){if(!value)throw new AssertionError("Native selector geometry");}
    public static void main(String[] args){
        int cases=0;
        for(int count=1;count<=8;count++)for(int item=50;item<=76;item+=2)
            for(int gap=-12;gap<=10;gap+=2)for(int inset=6;inset<=12;inset+=3){
                int rows=NativeSelectorGeometry.visibleRows(count,356,inset,inset,item,gap);
                check(rows>=1&&rows<=count);
                check(NativeSelectorGeometry.contentHeight(rows,item,gap)+2*inset<=356);
                if(rows<count)check(NativeSelectorGeometry.contentHeight(rows+1,item,gap)+2*inset>356);
                cases++;
            }
        check(NativeSelectorGeometry.contentHeight(4,50,10)+18==248); // observed OEM units popup
        check(NativeSelectorGeometry.contentHeight(4,56,4)+18==254); // App-Connect donor, not units hardcoding
        check(NativeSelectorGeometry.contentHeight(4,56,4)+12==248); // native App-Connect viewport padding + button y3
        check(NativeSelectorGeometry.contentHeight(4,56,4)+6+6==248); // actual SEAT list top padding6 + popup outer bottom6
        check(NativeSelectorGeometry.visibleRows(8,356,9,9,56,4)==5);
        check(NativeSelectorGeometry.contentHeight(4,72,-6)==270); // native older G11 overlap, pitch66
        check(NativeSelectorGeometry.visibleRows(8,365,7,0,72,-6)==5);
        check(NativeSelectorGeometry.popupHeight(0,242,0,6)==248); // SEAT unchanged
        check(NativeSelectorGeometry.popupHeight(21,277,25,5)==328); // Skoda four complete options
        check(NativeSelectorGeometry.popupHeight(21,343,25,5)==394); // Skoda five-row scrolling viewport
        for(int listY=0;listY<=24;listY+=3)for(int count=1;count<=8;count++) {
            int available=402-listY-25-5;
            int rows=NativeSelectorGeometry.visibleRows(count,available,7,0,72,-6);
            int viewport=NativeSelectorGeometry.contentHeight(rows,72,-6)+7;
            int outer=NativeSelectorGeometry.popupHeight(listY,viewport,25,5);
            check(listY+viewport<=outer&&outer<=402);cases++;
        }
        boolean badOffset=false;try{NativeSelectorGeometry.popupHeight(-1,277,25,5);}catch(IllegalArgumentException ok){badOffset=true;}
        check(badOffset);
        boolean badPitch=false;try{NativeSelectorGeometry.contentHeight(4,72,-72);}catch(IllegalArgumentException ok){badPitch=true;}
        check(badPitch);
        check(NativeSelectorGeometry.top(137,83,356,248)==137);
        check(NativeSelectorGeometry.top(66,83,356,308)==83);
        check(NativeSelectorGeometry.top(300,83,356,308)==131);
        for(int height=60;height<=350;height+=10)for(int nativeY=-60;nativeY<=540;nativeY+=20){
            int y=NativeSelectorGeometry.top(nativeY,83,356,height);
            check(y>=83&&y+height<=439);cases++;
        }
        boolean rejected=false;try{NativeSelectorGeometry.visibleRows(4,40,9,9,56,4);}catch(IllegalArgumentException ok){rejected=true;}
        check(rejected);
        System.out.println("NATIVE_SELECTOR_SKIN_BOUNDS_PASS cases="+cases);
    }
}

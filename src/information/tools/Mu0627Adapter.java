import java.nio.file.*;
import java.util.*;
import java.util.zip.*;
import jdk.internal.org.objectweb.asm.*;

/** Retarget V53.2 MU0841 to the supplied MU0627 firmware, without recompiling
 * unrelated media/navigation behavior or changing firmware files. */
public class Mu0627Adapter implements Opcodes {
  static final String NAV="de/vw/mib/bap/mqbab2/navsd/functions/InfoStates";
  static final String BRIDGE="com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge";
  static String map(String s) {
    if(s==null)return null;
    return s.replace("ActiveSource_ListAvailable","ActiveSource_Status$ListAvailable")
      .replace("CurrentStationInfo_StationInfoSwitches","CurrentStationInfo_Status$StationInfoSwitches")
      .replace("CurrentStationInfo_StationProperties","CurrentStationInfo_Status$StationProperties");
  }
  static byte[] adapt(byte[] bytes,byte[] stock) {
    ClassReader r=new ClassReader(bytes);String name=r.getClassName();
    Set<String> replace=new HashSet<>();
    if(name.endsWith("/RequestHandler"))replace.add("<init>");
    if(name.endsWith("/CarPlayModeHandling"))replace.add("<init>");
    if(name.equals(NAV))replace.addAll(Arrays.asList("init","uninitialize","getCurrentBAPInfoState"));
    ClassWriter w=new ClassWriter(ClassWriter.COMPUTE_MAXS);
    ClassVisitor cv=new ClassVisitor(ASM8,w){
      public void visit(int v,int a,String n,String s,String p,String[] it) {
        if(name.equals(NAV))it=Arrays.stream(it).filter(x->!x.endsWith("/CarLifeServiceListener")).toArray(String[]::new);
        super.visit(V1_4,a,n,map(s),map(p),it);
      }
      public FieldVisitor visitField(int a,String n,String d,String s,Object v){
        if(n.equals("exboxGuidanceListenerImpl"))return null;
        return super.visitField(a,n,map(d),map(s),v);
      }
      public MethodVisitor visitMethod(int a,String n,String d,String s,String[] ex){
        if(replace.contains(n)||n.equals("initExBoxNavServices")||n.equals("updateCarLifeData")||n.equals("requestUI2"))return null;
        MethodVisitor mv=super.visitMethod(a,n,map(d),map(s),ex);
        return new MethodVisitor(ASM8,mv){
          public void visitFieldInsn(int op,String owner,String field,String desc){
            if(owner.endsWith("CurrentStationInfo_StationInfoSwitches")&&field.equals("vicsAvailableJapanMarketOnly"))field="vicsAvailable";
            if(owner.endsWith("CurrentStationInfo_StationProperties")&&field.equals("ibocLiveTransmissionActiveHdLiveMode"))field="ibocLiveTransmissionActive";
            super.visitFieldInsn(op,map(owner),field,map(desc));
          }
          public void visitMethodInsn(int op,String o,String n,String d,boolean it){super.visitMethodInsn(op,map(o),n,map(d),it);}
          public void visitTypeInsn(int op,String type){super.visitTypeInsn(op,map(type));}
          public void visitLdcInsn(Object v){if(v instanceof Type)v=Type.getType(map(((Type)v).getDescriptor()));super.visitLdcInsn(v);}
          public void visitLocalVariable(String n,String d,String s,Label start,Label end,int index){super.visitLocalVariable(n,map(d),map(s),start,end,index);}
          public void visitFrame(int t,int nl,Object[] l,int ns,Object[] s){} // version 48 verifier computes frames
        };
      }
    };
    r.accept(cv,ClassReader.SKIP_FRAMES);
    if(!replace.isEmpty()) {
      if(stock==null)throw new IllegalStateException("stock input missing: "+name);
      new ClassReader(stock).accept(new ClassVisitor(ASM8){
        public MethodVisitor visitMethod(int a,String n,String d,String s,String[] ex){
          if(!replace.contains(n))return null;
          MethodVisitor mv=w.visitMethod(a,n,d,s,ex);
          return new MethodVisitor(ASM8,mv){
            public void visitCode(){
              super.visitCode();
              if(name.equals(NAV)&&n.equals("init")){
                mv.visitVarInsn(ALOAD,0);mv.visitFieldInsn(PUTSTATIC,NAV,"INSTANCE","L"+NAV+";");
                Label start=new Label(),end=new Label(),handler=new Label(),after=new Label();
                mv.visitTryCatchBlock(start,end,handler,"java/lang/Throwable");mv.visitLabel(start);
                mv.visitLdcInsn("InfoStates");mv.visitVarInsn(ALOAD,0);
                mv.visitMethodInsn(INVOKESTATIC,BRIDGE,"markBapShadowInit","(Ljava/lang/String;Ljava/lang/Object;)V",false);
                mv.visitLabel(end);mv.visitJumpInsn(GOTO,after);mv.visitLabel(handler);mv.visitInsn(POP);mv.visitLabel(after);
              }
              if(name.equals(NAV)&&n.equals("uninitialize")){
                Label after=new Label();mv.visitFieldInsn(GETSTATIC,NAV,"INSTANCE","L"+NAV+";");mv.visitVarInsn(ALOAD,0);mv.visitJumpInsn(IF_ACMPNE,after);mv.visitInsn(ACONST_NULL);mv.visitFieldInsn(PUTSTATIC,NAV,"INSTANCE","L"+NAV+";");mv.visitLabel(after);
              }
            }
            public void visitFrame(int t,int nl,Object[] l,int ns,Object[] s){}
          };
        }
      },ClassReader.SKIP_FRAMES|ClassReader.SKIP_DEBUG);
    }
    return w.toByteArray();
  }
  public static void main(String[] args)throws Exception {
    try(ZipFile base=new ZipFile(args[0]);ZipFile stock=new ZipFile(args[1]);ZipOutputStream out=new ZipOutputStream(Files.newOutputStream(Paths.get(args[2])))){
      List<? extends ZipEntry> entries=Collections.list(base.entries());entries.sort(Comparator.comparing(ZipEntry::getName));
      for(ZipEntry e:entries){
        if(e.getName().equals("de/vw/mib/asl/internal/exboxm/api/impl/guidance/ExboxGuidanceManagerImpl.class"))continue;
        byte[] b=base.getInputStream(e).readAllBytes();
        if(e.getName().endsWith(".class")){
          ZipEntry s=stock.getEntry(e.getName());b=adapt(b,s==null?null:stock.getInputStream(s).readAllBytes());
        }
        ZipEntry ne=new ZipEntry(e.getName());ne.setTime(315532802000L);out.putNextEntry(ne);out.write(b);out.closeEntry();
      }
    }
    System.out.println("MU0627 adapter complete: "+args[2]);
  }
}

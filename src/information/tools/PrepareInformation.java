import java.nio.file.*;
import java.util.*;
import java.util.jar.*;
import jdk.internal.org.objectweb.asm.*;

/** Isolate information from the already-audited navigation/video engines. */
public final class PrepareInformation implements Opcodes {
    static final Set<String> NAVIGATION_CLASSES=new HashSet<String>(Arrays.asList(
        "de/esolutions/fw/comm/dsi/carplay/impl/DSICarplayProxy.class",
        "de/esolutions/fw/dsi/androidauto2/DSIAndroidAuto2Dispatcher.class",
        "de/esolutions/fw/dsi/androidauto/DSIAndroidAutoDispatcher.class",
        "de/vw/mib/asl/internal/androidauto/target/NavigationListener.class",
        "de/vw/mib/asl/internal/androidauto/target/RequestHandler.class",
        "de/vw/mib/asl/internal/carplay/common/CarPlayModeHandling.class",
        "de/vw/mib/asl/internal/exboxm/api/impl/guidance/ExboxGuidanceManagerImpl.class",
        "de/vw/mib/asl/internal/navigation/api/impl/ASLNavigationServices.class"));
    static int timezone,bluetooth,getClassFix,bootstrap,dispatch;
    static String runtimePath(String value) {
        if(value.contains("/mnt/app/root/lib-target/cpvc-rgd-bootstrap.sh"))bootstrap++;
        return value.replace("/mnt/app/root/lib-target/","/mnt/app/carplaymaps/information/")
            .replace("/mnt/media/cpvc-runtime/","/ramdisk/cpmap-info/runtime/")
            .replace("/var/app/icab/tmp/cp_cover","/ramdisk/cpmap-info/cp_cover")
            .replace("/mnt/icab/persistent/carplayvc/","/ramdisk/cpmap-info/");
    }
    static byte[] patch(byte[] bytes,final boolean europe) {
        ClassReader reader=new ClassReader(bytes);
        final String name=reader.getClassName();
        ClassWriter writer=new ClassWriter(ClassWriter.COMPUTE_MAXS);
        reader.accept(new ClassVisitor(ASM8,writer) {
            public FieldVisitor visitField(int a,String n,String d,String s,Object value) {
                if(value instanceof String)value=runtimePath((String)value);
                return super.visitField(a,n,d,s,value);
            }
            public MethodVisitor visitMethod(int a,String n,String d,String s,String[] e) {
                MethodVisitor next=super.visitMethod(a,n,d,s,e);
                // Public scope is CarPlay information. Do not start V53's
                // optional local-Bluetooth listener or replace OEM BT metadata.
                if(name.equals("com/mst2/carplay/bridge/Mst2LocalBtNowPlayingBridge")
                    && n.equals("attachOnce")&&d.endsWith(")V")) {
                    bluetooth++;next.visitCode();next.visitInsn(RETURN);
                    next.visitMaxs(0,1);next.visitEnd();return null;
                }
                return new MethodVisitor(ASM8,next) {
                    public void visitCode() {
                        super.visitCode();
                        if(name.equals("com/mst2/carplay/bridge/Mst2NowPlayingBridge")
                            &&n.equals("tryAttach")&&d.equals("(Lde/vw/mib/asl/internal/carplay/target/HsmTarget;)Z")) {
                            dispatch++;super.visitVarInsn(ALOAD,0);
                            super.visitMethodInsn(INVOKESTATIC,"org/omonob/cpmap/information/InformationAslDispatch","attach","(Ljava/lang/Object;)I",false);
                            super.visitInsn(DUP);super.visitInsn(ICONST_M1);
                            Label owning=new Label();super.visitJumpInsn(IF_ICMPEQ,owning);
                            super.visitInsn(IRETURN);super.visitLabel(owning);super.visitInsn(POP);
                        }
                        if(name.equals("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge")) {
                            int operation=n.equals("sendInitVolley")&&d.equals("()V")?0:n.equals("sendCleanupVolley")&&d.equals("()V")?1:n.equals("sendVcFrame")&&d.equals("()V")?2:n.equals("triggerBapShadows")&&d.equals("(Ljava/lang/String;)V")?3:-1;
                            if(operation>=0) {
                                dispatch++;super.visitLdcInsn(operation);
                                if(operation==3)super.visitVarInsn(ALOAD,0);else super.visitInsn(ACONST_NULL);
                                super.visitMethodInsn(INVOKESTATIC,"org/omonob/cpmap/information/InformationAslDispatch","defer","(ILjava/lang/String;)Z",false);
                                Label now=new Label();super.visitJumpInsn(IFEQ,now);super.visitInsn(RETURN);super.visitLabel(now);
                                if(operation==2) {
                                    super.visitMethodInsn(INVOKESTATIC,"org/omonob/cpmap/ui/ProjectionRuntimePolicy","navigationAllowed","()Z",false);
                                    Label allowed=new Label();super.visitJumpInsn(IFNE,allowed);super.visitInsn(RETURN);super.visitLabel(allowed);
                                }
                            }
                        }
                    }
                    public void visitLdcInsn(Object value) {
                        if(value instanceof String)value=runtimePath((String)value);
                        super.visitLdcInsn(value);
                    }
                    public void visitMethodInsn(int op,String owner,String method,String desc,boolean itf) {
                        // The historical CN implementation hard-coded Shanghai
                        // in two ETA calculations. European firmware must use
                        // its local system timezone, including its DST rules.
                        if(europe&&name.equals("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge")
                            &&owner.equals("java/util/TimeZone")&&method.equals("getTimeZone")
                            &&desc.equals("(Ljava/lang/String;)Ljava/util/TimeZone;")) {
                            timezone++;super.visitInsn(POP);
                            super.visitMethodInsn(INVOKESTATIC,owner,"getDefault","()Ljava/util/TimeZone;",false);return;
                        }
                        // getClass is final. Correct the known JXE->JAR opcode
                        // form without changing the selected method or receiver.
                        if(op==INVOKESPECIAL&&owner.equals("java/lang/Object")
                            &&method.equals("getClass")&&desc.equals("()Ljava/lang/Class;")) {
                            getClassFix++;op=INVOKEVIRTUAL;
                        }
                        super.visitMethodInsn(op,owner,method,desc,itf);
                    }
                };
            }
        },ClassReader.SKIP_FRAMES);
        return writer.toByteArray();
    }
    public static void main(String[] args)throws Exception {
        String mu=args[2];boolean europe=!mu.equals("MU1102");int kept=0,removed=0;
        if(!Arrays.asList("MU1102","MU1367","MU1440","MU1433","MU1447").contains(mu))
            throw new IllegalArgumentException("Unsupported information firmware");
        try(JarFile source=new JarFile(args[0]);JarOutputStream jar=new JarOutputStream(Files.newOutputStream(Path.of(args[1])))) {
            List<JarEntry> entries=Collections.list(source.entries());
            entries.sort(Comparator.comparing(JarEntry::getName));
            for(JarEntry entry:entries) {
                if(!entry.getName().endsWith(".class"))continue;
                if(NAVIGATION_CLASSES.contains(entry.getName())){removed++;continue;}
                JarEntry copy=new JarEntry(entry.getName());copy.setTime(315532802000L);
                jar.putNextEntry(copy);jar.write(patch(source.getInputStream(entry).readAllBytes(),europe));
                jar.closeEntry();kept++;
            }
        }
        if(bluetooth!=1||timezone!=(europe?2:0)||bootstrap<1||kept!=41||dispatch!=5)
            throw new IllegalStateException("Unexpected scope: kept="+kept+" bt="+bluetooth+" timezone="+timezone+" bootstrap="+bootstrap);
        System.out.println("INFORMATION_SCOPE_PASS firmware="+mu+" classes="+kept+" excluded_navigation_classes="+removed
            +" asl_thread_dispatch="+dispatch+" bluetooth_listener_started=0 eta_timezone="+(europe?"system-default":"Asia/Shanghai")+" jxe_getclass_corrections="+getClassFix);
    }
}

import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.jar.*;
import java.io.*;
import jdk.internal.org.objectweb.asm.*;

/** Offline exact-JAR experiment; copies the input, never edits the formal JAR. */
public final class BuildV53PolicyHook {
    static final String POLICY="org/omonob/cpmap/ui/ProjectionRuntimePolicy";
    static int navigation,music,cover,navTransition,coverTick;
    static void emitCoverTick(MethodVisitor v,String owner){
        Label normal=new Label(),clearDone=new Label(),original=new Label();
        v.visitMethodInsn(Opcodes.INVOKESTATIC,POLICY,"musicAllowed","()Z",false);v.visitJumpInsn(Opcodes.IFNE,normal);
        v.visitMethodInsn(Opcodes.INVOKESTATIC,owner,"mst2BuildLiveRL","()Lorg/dsi/ifc/global/ResourceLocator;",false);v.visitVarInsn(Opcodes.ASTORE,1);
        v.visitVarInsn(Opcodes.ALOAD,1);v.visitJumpInsn(Opcodes.IFNULL,normal);
        v.visitVarInsn(Opcodes.ALOAD,1);v.visitMethodInsn(Opcodes.INVOKESTATIC,POLICY,"filterCover","(Lorg/dsi/ifc/global/ResourceLocator;)Lorg/dsi/ifc/global/ResourceLocator;",false);v.visitJumpInsn(Opcodes.IFNONNULL,normal);
        v.visitVarInsn(Opcodes.ALOAD,0);v.visitFieldInsn(Opcodes.GETFIELD,owner,"cpvcMenuCoverHidden","Z");v.visitJumpInsn(Opcodes.IFNE,clearDone);
        v.visitVarInsn(Opcodes.ALOAD,0);v.visitInsn(Opcodes.ICONST_1);v.visitFieldInsn(Opcodes.PUTFIELD,owner,"cpvcMenuCoverHidden","Z");
        v.visitVarInsn(Opcodes.ALOAD,0);v.visitInsn(Opcodes.ACONST_NULL);v.visitMethodInsn(Opcodes.INVOKESPECIAL,owner,"_transmitCoverArt","(Lorg/dsi/ifc/global/ResourceLocator;)V",false);
        v.visitLabel(clearDone);v.visitInsn(Opcodes.RETURN);
        v.visitLabel(normal);v.visitVarInsn(Opcodes.ALOAD,0);v.visitFieldInsn(Opcodes.GETFIELD,owner,"cpvcMenuCoverHidden","Z");v.visitJumpInsn(Opcodes.IFEQ,original);
        v.visitVarInsn(Opcodes.ALOAD,0);v.visitInsn(Opcodes.ICONST_0);v.visitFieldInsn(Opcodes.PUTFIELD,owner,"cpvcMenuCoverHidden","Z");
        for(String field:new String[]{"mst2LastPushedUrl","mst2LastPushedDaemonHash","cpvcCoverOnceLastKey"}){v.visitInsn(Opcodes.ACONST_NULL);v.visitFieldInsn(Opcodes.PUTSTATIC,owner,field,"Ljava/lang/String;");}
        v.visitLabel(original);
    }
    static byte[] patch(String name,byte[] input) {
        ClassReader reader=new ClassReader(input);ClassWriter writer=new ClassWriter(ClassWriter.COMPUTE_MAXS);
        reader.accept(new ClassVisitor(Opcodes.ASM8,writer){
            public void visitEnd(){
                if(name.equals("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge.class"))
                    super.visitField(Opcodes.ACC_PRIVATE|Opcodes.ACC_STATIC,"cpvcMenuNavWasOff","Z",null,null).visitEnd();
                if(name.equals("de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt.class"))
                    super.visitField(Opcodes.ACC_PRIVATE,"cpvcMenuCoverHidden","Z",null,null).visitEnd();
                super.visitEnd();
            }
            public MethodVisitor visitMethod(int access,String method,String desc,String sig,String[] exceptions){
                MethodVisitor next=super.visitMethod(access,method,desc,sig,exceptions);
                boolean nav=name.equals("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge.class") && method.equals("isOverrideActive") && desc.equals("()Z");
                boolean media=name.startsWith("de/vw/mib/bap/mqbab2/audiosd/functions/CurrentStationInfo") && method.equals("setStationInfoForMirrorLink");
                return new MethodVisitor(Opcodes.ASM8,next){
                    public void visitCode(){
                        super.visitCode();
                        if(nav){
                            navigation++;
                            super.visitMethodInsn(Opcodes.INVOKESTATIC,POLICY,"navigationAllowed","()Z",false);
                            Label allowed=new Label();super.visitJumpInsn(Opcodes.IFNE,allowed);
                            super.visitInsn(Opcodes.ICONST_0);super.visitInsn(Opcodes.IRETURN);super.visitLabel(allowed);
                        }
                        if(name.equals("de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt.class") && method.equals("_transmitCoverArt") && desc.equals("(Lorg/dsi/ifc/global/ResourceLocator;)V")){
                            cover++;super.visitVarInsn(Opcodes.ALOAD,1);
                            super.visitMethodInsn(Opcodes.INVOKESTATIC,POLICY,"filterCover","(Lorg/dsi/ifc/global/ResourceLocator;)Lorg/dsi/ifc/global/ResourceLocator;",false);
                            super.visitVarInsn(Opcodes.ASTORE,1);
                        }
                        if(name.equals("de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt.class") && method.equals("mst2TickPush") && desc.equals("()V")){
                            // Reuse the existing ticker, clearing once even if
                            // the song hash stays identical across the toggle.
                            coverTick++;emitCoverTick(mv,name.substring(0,name.length()-6));
                        }
                        if(name.equals("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge.class") && method.equals("shouldWriteNow") && desc.equals("()Z")){
                            navTransition++;String owner=name.substring(0,name.length()-6);
                            Label enabled=new Label(),alreadyCleared=new Label(),resume=new Label();
                            super.visitMethodInsn(Opcodes.INVOKESTATIC,POLICY,"navigationAllowed","()Z",false);
                            super.visitJumpInsn(Opcodes.IFNE,enabled);
                            super.visitFieldInsn(Opcodes.GETSTATIC,owner,"cpvcMenuNavWasOff","Z");super.visitJumpInsn(Opcodes.IFNE,alreadyCleared);
                            super.visitInsn(Opcodes.ICONST_1);super.visitFieldInsn(Opcodes.PUTSTATIC,owner,"cpvcMenuNavWasOff","Z");
                            super.visitMethodInsn(Opcodes.INVOKESTATIC,owner,"sendCleanupVolley","()V",false);
                            super.visitLabel(alreadyCleared);super.visitInsn(Opcodes.ICONST_0);super.visitInsn(Opcodes.IRETURN);
                            super.visitLabel(enabled);super.visitFieldInsn(Opcodes.GETSTATIC,owner,"cpvcMenuNavWasOff","Z");super.visitJumpInsn(Opcodes.IFEQ,resume);
                            super.visitInsn(Opcodes.ICONST_0);super.visitFieldInsn(Opcodes.PUTSTATIC,owner,"cpvcMenuNavWasOff","Z");
                            super.visitLdcInsn("menu-navigation-enabled");super.visitMethodInsn(Opcodes.INVOKESTATIC,owner,"triggerBapShadows","(Ljava/lang/String;)V",false);
                            super.visitLabel(resume);
                        }
                    }
                    public void visitMethodInsn(int opcode,String owner,String call,String descriptor,boolean itf){
                        super.visitMethodInsn(opcode,owner,call,descriptor,itf);
                        // Reuse the original CarPlay/call-active branch which
                        // sends NoInformationAvailable, rather than silently
                        // retaining stale text or interfering with radio/BT.
                        if(media && opcode==Opcodes.INVOKESTATIC && owner.equals("com/mst2/carplay/bridge/Mst2NowPlayingBridge") && call.equals("isCallActive") && descriptor.equals("()Z")){
                            music++;super.visitMethodInsn(Opcodes.INVOKESTATIC,POLICY,"callActiveOrMusicHidden","(Z)Z",false);
                        }
                    }
                };
            }
        },ClassReader.SKIP_FRAMES);
        return writer.toByteArray();
    }
    public static void main(String[] args)throws Exception {
        Path baseline=Path.of(args[0]), module=Path.of(args[1]), output=Path.of(args[2]);
        Files.createDirectories(output.getParent());Path temp=Files.createTempFile(output.getParent(),"v53-policy-",".tmp");
        int changed=0;
        try(JarFile source=new JarFile(baseline.toFile());JarOutputStream target=new JarOutputStream(Files.newOutputStream(temp))){
            Enumeration<JarEntry> entries=source.entries();
            while(entries.hasMoreElements()){
                JarEntry entry=entries.nextElement();byte[] bytes=source.getInputStream(entry).readAllBytes();String name=entry.getName();
                if(name.equals("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge.class") || name.equals("de/vw/mib/bap/mqbab2/audiosd/functions/CurrentStationInfo.class") || name.equals("de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt.class")){
                    byte[] updated=patch(name,bytes);if(!Arrays.equals(bytes,updated))changed++;bytes=updated;
                }
                JarEntry copy=new JarEntry(name);copy.setTime(0);target.putNextEntry(copy);target.write(bytes);target.closeEntry();
            }
            try(JarFile menu=new JarFile(module.toFile())){
                Enumeration<JarEntry> menuEntries=menu.entries();
                while(menuEntries.hasMoreElements()){
                    JarEntry entry=menuEntries.nextElement();String name=entry.getName();
                    if(!(name.startsWith("org/omonob/cpmap/ui/ProjectionSettings") || name.startsWith("org/omonob/cpmap/ui/ProjectionRuntimePolicy") || name.startsWith("org/omonob/cpmap/ui/MapSwitchSettings")))continue;
                    JarEntry copy=new JarEntry(name);copy.setTime(0);target.putNextEntry(copy);target.write(menu.getInputStream(entry).readAllBytes());target.closeEntry();
                }
            }
        }
        // ChinaJp inherits the base CarPlay branch; it needs no duplicate hook.
        if(navigation!=1 || music!=1 || cover!=1 || navTransition!=1 || coverTick!=1 || changed!=3)throw new IllegalStateException("Exact V53 hook ABI mismatch nav="+navigation+" music="+music+" cover="+cover+" navTransition="+navTransition+" coverTick="+coverTick+" classes="+changed);
        Files.move(temp,output,StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);
        System.out.println("V53_POLICY_HOOK_BUILT navigation="+navigation+" music="+music+" cover="+cover+" transition_cleanup="+navTransition+" unchanged_track_cover_transition="+coverTick+" changed_stock_classes="+changed);
    }
}

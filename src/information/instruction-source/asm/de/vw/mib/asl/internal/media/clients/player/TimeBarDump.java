// Historical V53.3 patch: reproducible instruction-level source.
// Generated locally from the author-supplied patch, not a full OEM JXE.
package asm.de.vw.mib.asl.internal.media.clients.player;
import jdk.internal.org.objectweb.asm.AnnotationVisitor;
import jdk.internal.org.objectweb.asm.Attribute;
import jdk.internal.org.objectweb.asm.ClassReader;
import jdk.internal.org.objectweb.asm.ClassWriter;
import jdk.internal.org.objectweb.asm.ConstantDynamic;
import jdk.internal.org.objectweb.asm.FieldVisitor;
import jdk.internal.org.objectweb.asm.Handle;
import jdk.internal.org.objectweb.asm.Label;
import jdk.internal.org.objectweb.asm.MethodVisitor;
import jdk.internal.org.objectweb.asm.Opcodes;
import jdk.internal.org.objectweb.asm.RecordComponentVisitor;
import jdk.internal.org.objectweb.asm.Type;
import jdk.internal.org.objectweb.asm.TypePath;
public class TimeBarDump implements Opcodes {

public static byte[] dump () throws Exception {

ClassWriter classWriter = new ClassWriter(0);
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;

classWriter.visit(V1_2, ACC_PUBLIC | ACC_SUPER, "de/vw/mib/asl/internal/media/clients/player/TimeBar", null, "java/lang/Object", null);

member_0(classWriter);
member_1(classWriter);
classWriter.visitEnd();

return classWriter.toByteArray();
}
private static void member_0(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "<init>", "()V", null, null);
methodVisitor.visitCode();
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKESPECIAL, "java/lang/Object", "<init>", "()V", false);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitInsn(NOP);
methodVisitor.visitInsn(NOP);
methodVisitor.visitInsn(NOP);
methodVisitor.visitMaxs(1, 1);
methodVisitor.visitEnd();
}
private static void member_1(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC | ACC_STATIC, "update", "(Lde/vw/mib/asl/internal/media/playback/MediaPlayerState;)V", null, null);
methodVisitor.visitCode();
methodVisitor.visitInsn(ACONST_NULL);
methodVisitor.visitVarInsn(ASTORE, 1);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitVarInsn(ISTORE, 2);
methodVisitor.visitVarInsn(ALOAD, 0);
Label label0 = new Label();
methodVisitor.visitJumpInsn(IFNULL, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/media/playback/MediaPlayerState", "getPlayTime", "()Lde/vw/mib/asl/internal/media/playback/PlayTime;", false);
methodVisitor.visitVarInsn(ASTORE, 1);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/media/playback/MediaPlayerState", "isActivated", "()Z", false);
methodVisitor.visitVarInsn(ISTORE, 2);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitMethodInsn(INVOKESTATIC, "com/mst2/carplay/bridge/Mst2LocalBtNowPlayingBridge", "getStablePlayTime", "(Lde/vw/mib/asl/internal/media/playback/MediaPlayerState;Lde/vw/mib/asl/internal/media/playback/PlayTime;)Lde/vw/mib/asl/internal/media/playback/PlayTime;", false);
methodVisitor.visitVarInsn(ASTORE, 1);
methodVisitor.visitLabel(label0);
methodVisitor.visitVarInsn(ILOAD, 2);
Label label1 = new Label();
methodVisitor.visitJumpInsn(IFEQ, label1);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitJumpInsn(IFNULL, label1);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/media/playback/PlayTime", "getTotalTime", "()I", false);
methodVisitor.visitVarInsn(ISTORE, 3);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/media/playback/PlayTime", "getPlayTime", "()I", false);
methodVisitor.visitVarInsn(ISTORE, 4);
methodVisitor.visitVarInsn(ILOAD, 3);
methodVisitor.visitVarInsn(ILOAD, 4);
Label label2 = new Label();
methodVisitor.visitJumpInsn(IF_ICMPLT, label2);
methodVisitor.visitVarInsn(ILOAD, 3);
methodVisitor.visitIntInsn(SIPUSH, 1000);
methodVisitor.visitInsn(IDIV);
methodVisitor.visitVarInsn(ILOAD, 4);
methodVisitor.visitIntInsn(SIPUSH, 1000);
methodVisitor.visitInsn(IDIV);
methodVisitor.visitInsn(ISUB);
methodVisitor.visitIntInsn(SIPUSH, 1000);
methodVisitor.visitInsn(IMUL);
Label label3 = new Label();
methodVisitor.visitJumpInsn(GOTO, label3);
methodVisitor.visitLabel(label2);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitLabel(label3);
methodVisitor.visitVarInsn(ISTORE, 5);
methodVisitor.visitFieldInsn(GETSTATIC, "de/vw/mib/asl/framework/internal/framework/ServiceManager", "aslPropertyManager", "Lde/vw/mib/asl/ASLPropertyManager;");
methodVisitor.visitIntInsn(BIPUSH, 14);
methodVisitor.visitVarInsn(ILOAD, 5);
methodVisitor.visitMethodInsn(INVOKEINTERFACE, "de/vw/mib/asl/ASLPropertyManager", "valueChangedInteger", "(II)V", true);
methodVisitor.visitFieldInsn(GETSTATIC, "de/vw/mib/asl/framework/internal/framework/ServiceManager", "aslPropertyManager", "Lde/vw/mib/asl/ASLPropertyManager;");
methodVisitor.visitIntInsn(BIPUSH, 13);
methodVisitor.visitVarInsn(ILOAD, 4);
methodVisitor.visitMethodInsn(INVOKEINTERFACE, "de/vw/mib/asl/ASLPropertyManager", "valueChangedInteger", "(II)V", true);
Label label4 = new Label();
methodVisitor.visitJumpInsn(GOTO, label4);
methodVisitor.visitLabel(label1);
methodVisitor.visitFieldInsn(GETSTATIC, "de/vw/mib/asl/framework/internal/framework/ServiceManager", "aslPropertyManager", "Lde/vw/mib/asl/ASLPropertyManager;");
methodVisitor.visitIntInsn(BIPUSH, 14);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitMethodInsn(INVOKEINTERFACE, "de/vw/mib/asl/ASLPropertyManager", "valueChangedInteger", "(II)V", true);
methodVisitor.visitFieldInsn(GETSTATIC, "de/vw/mib/asl/framework/internal/framework/ServiceManager", "aslPropertyManager", "Lde/vw/mib/asl/ASLPropertyManager;");
methodVisitor.visitIntInsn(BIPUSH, 13);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitMethodInsn(INVOKEINTERFACE, "de/vw/mib/asl/ASLPropertyManager", "valueChangedInteger", "(II)V", true);
methodVisitor.visitLabel(label4);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitInsn(NOP);
methodVisitor.visitMaxs(3, 6);
methodVisitor.visitEnd();
}
}

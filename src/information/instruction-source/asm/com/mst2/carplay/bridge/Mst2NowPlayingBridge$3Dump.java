// Historical V53.3 patch: reproducible instruction-level source.
// Generated locally from the author-supplied patch, not a full OEM JXE.
package asm.com.mst2.carplay.bridge;
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
public class Mst2NowPlayingBridge$3Dump implements Opcodes {

public static byte[] dump () throws Exception {

ClassWriter classWriter = new ClassWriter(0);
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;

classWriter.visit(V1_4, ACC_FINAL | ACC_SUPER, "com/mst2/carplay/bridge/Mst2NowPlayingBridge$3", null, "java/lang/Object", new String[] { "java/lang/Runnable" });

classWriter.visitSource("Mst2NowPlayingBridge.java", null);

classWriter.visitInnerClass("com/mst2/carplay/bridge/Mst2NowPlayingBridge$3", null, null, ACC_STATIC);

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
methodVisitor = classWriter.visitMethod(0, "<init>", "()V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(1009, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKESPECIAL, "java/lang/Object", "<init>", "()V", false);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(1, 1);
methodVisitor.visitEnd();
}
private static void member_1(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "run", "()V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(1011, label0);
methodVisitor.visitLdcInsn("shutdown-hook: bridge cleanup");
methodVisitor.visitMethodInsn(INVOKESTATIC, "com/mst2/carplay/bridge/Mst2NowPlayingBridge", "access$300", "(Ljava/lang/String;)V", false);
Label label1 = new Label();
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(1012, label1);
methodVisitor.visitMethodInsn(INVOKESTATIC, "com/mst2/carplay/bridge/Mst2NowPlayingBridge", "resetAttach", "()V", false);
Label label2 = new Label();
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(1013, label2);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(1, 1);
methodVisitor.visitEnd();
}
}

// Historical V53.3 patch: reproducible instruction-level source.
// Generated locally from the author-supplied patch, not a full OEM JXE.
package asm.com.mst2.carplay.navi;
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
public class Mst2CarPlayNaviSinkBridge$4Dump implements Opcodes {

public static byte[] dump () throws Exception {

ClassWriter classWriter = new ClassWriter(0);
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;

classWriter.visit(V1_4, ACC_FINAL | ACC_SUPER, "com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge$4", null, "java/lang/Object", new String[] { "java/lang/Runnable" });

classWriter.visitSource("Mst2CarPlayNaviSinkBridge.java", null);

classWriter.visitInnerClass("com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge$4", null, null, ACC_STATIC);

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
methodVisitor.visitLineNumber(2866, label0);
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
methodVisitor.visitLineNumber(2868, label0);
methodVisitor.visitLdcInsn("shutdown-hook: bridge cleanup");
methodVisitor.visitMethodInsn(INVOKESTATIC, "com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge", "log", "(Ljava/lang/String;)V", false);
Label label1 = new Label();
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(2869, label1);
methodVisitor.visitMethodInsn(INVOKESTATIC, "com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge", "access$3400", "()V", false);
Label label2 = new Label();
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(2870, label2);
methodVisitor.visitMethodInsn(INVOKESTATIC, "com/mst2/carplay/navi/Mst2CarPlayNaviSinkBridge", "resetAttach", "()V", false);
Label label3 = new Label();
methodVisitor.visitLabel(label3);
methodVisitor.visitLineNumber(2871, label3);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(1, 1);
methodVisitor.visitEnd();
}
}

// Historical V53.3 patch: reproducible instruction-level source.
// Generated locally from the author-supplied patch, not a full OEM JXE.
package asm.de.vw.mib.asl.internal.kombipictureserver.usecaces;
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
public class CoverArt$1Dump implements Opcodes {

public static byte[] dump () throws Exception {

ClassWriter classWriter = new ClassWriter(0);
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;

classWriter.visit(V1_4, ACC_SUPER, "de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt$1", null, "java/lang/Object", new String[] { "java/lang/Runnable" });

classWriter.visitSource("CoverArt.java", null);

classWriter.visitInnerClass("de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt$1", null, null, 0);

member_0(classWriter);
member_1(classWriter);
member_2(classWriter);
member_3(classWriter);
classWriter.visitEnd();

return classWriter.toByteArray();
}
private static void member_0(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE | ACC_FINAL | ACC_SYNTHETIC, "val$self", "Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;", null, null);
fieldVisitor.visitEnd();
}
private static void member_1(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE | ACC_FINAL | ACC_SYNTHETIC, "this$0", "Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;", null, null);
fieldVisitor.visitEnd();
}
private static void member_2(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(0, "<init>", "(Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;)V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(527, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt$1", "this$0", "Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;");
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 2);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt$1", "val$self", "Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;");
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKESPECIAL, "java/lang/Object", "<init>", "()V", false);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(2, 3);
methodVisitor.visitEnd();
}
private static void member_3(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "run", "()V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
Label label1 = new Label();
Label label2 = new Label();
methodVisitor.visitTryCatchBlock(label0, label1, label2, "java/lang/InterruptedException");
Label label3 = new Label();
methodVisitor.visitTryCatchBlock(label0, label1, label3, "java/lang/Throwable");
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(531, label0);
methodVisitor.visitLdcInsn(new Long(250L));
methodVisitor.visitMethodInsn(INVOKESTATIC, "java/lang/Thread", "sleep", "(J)V", false);
Label label4 = new Label();
methodVisitor.visitLabel(label4);
methodVisitor.visitLineNumber(532, label4);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitFieldInsn(GETFIELD, "de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt$1", "val$self", "Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;");
methodVisitor.visitMethodInsn(INVOKESTATIC, "de/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt", "access$000", "(Lde/vw/mib/asl/internal/kombipictureserver/usecaces/CoverArt;)V", false);
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(537, label1);
methodVisitor.visitJumpInsn(GOTO, label0);
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(533, label2);
methodVisitor.visitVarInsn(ASTORE, 1);
Label label5 = new Label();
methodVisitor.visitLabel(label5);
methodVisitor.visitLineNumber(534, label5);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitLabel(label3);
methodVisitor.visitLineNumber(535, label3);
methodVisitor.visitVarInsn(ASTORE, 1);
Label label6 = new Label();
methodVisitor.visitLabel(label6);
methodVisitor.visitLineNumber(537, label6);
methodVisitor.visitJumpInsn(GOTO, label0);
methodVisitor.visitMaxs(2, 2);
methodVisitor.visitEnd();
}
}

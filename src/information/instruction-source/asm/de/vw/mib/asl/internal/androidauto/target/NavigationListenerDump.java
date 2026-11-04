// Historical V53.3 patch: reproducible instruction-level source.
// Generated locally from the author-supplied patch, not a full OEM JXE.
package asm.de.vw.mib.asl.internal.androidauto.target;
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
public class NavigationListenerDump implements Opcodes {

public static byte[] dump () throws Exception {

ClassWriter classWriter = new ClassWriter(0);
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;

classWriter.visit(V1_2, ACC_PUBLIC | ACC_SUPER, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", null, "de/vw/mib/asl/api/navigation/AbstractASLNavigationServicesListener", null);

classWriter.visitSource("NavigationListener.java", null);

member_0(classWriter);
member_1(classWriter);
member_2(classWriter);
member_3(classWriter);
member_4(classWriter);
member_5(classWriter);
member_6(classWriter);
member_7(classWriter);
member_8(classWriter);
member_9(classWriter);
member_10(classWriter);
classWriter.visitEnd();

return classWriter.toByteArray();
}
private static void member_0(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE, "target", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;", null, null);
fieldVisitor.visitEnd();
}
private static void member_1(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE, "startupHandler", "Lde/vw/mib/asl/internal/androidauto/target/StartupHandler;", null, null);
fieldVisitor.visitEnd();
}
private static void member_2(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE, "properties", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoGlobalProperties;", null, null);
fieldVisitor.visitEnd();
}
private static void member_3(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE, "navServices", "Lde/vw/mib/asl/api/navigation/ASLNavigationServices;", null, null);
fieldVisitor.visitEnd();
}
private static void member_4(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
fieldVisitor = classWriter.visitField(ACC_PRIVATE, "stopGuidancePending", "Z", null, null);
fieldVisitor.visitEnd();
}
private static void member_5(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "<init>", "(Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoGlobalProperties;Lde/vw/mib/asl/internal/androidauto/target/StartupHandler;)V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(21, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKESPECIAL, "de/vw/mib/asl/api/navigation/AbstractASLNavigationServicesListener", "<init>", "()V", false);
Label label1 = new Label();
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(19, label1);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "stopGuidancePending", "Z");
Label label2 = new Label();
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(22, label2);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "target", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;");
Label label3 = new Label();
methodVisitor.visitLabel(label3);
methodVisitor.visitLineNumber(23, label3);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 2);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "properties", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoGlobalProperties;");
Label label4 = new Label();
methodVisitor.visitLabel(label4);
methodVisitor.visitLineNumber(24, label4);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 3);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "startupHandler", "Lde/vw/mib/asl/internal/androidauto/target/StartupHandler;");
Label label5 = new Label();
methodVisitor.visitLabel(label5);
methodVisitor.visitLineNumber(25, label5);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(2, 4);
methodVisitor.visitEnd();
}
private static void member_6(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "initNavServices", "(Lde/vw/mib/asl/api/navigation/ASLNavigationServices;)V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(28, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitVarInsn(ALOAD, 1);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "navServices", "Lde/vw/mib/asl/api/navigation/ASLNavigationServices;");
Label label1 = new Label();
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(29, label1);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(2, 2);
methodVisitor.visitEnd();
}
private static void member_7(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "updateGuidanceActive", "(Z)V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(33, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitFieldInsn(GETFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "target", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;");
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget", "isTraceEnabled", "()Z", false);
Label label1 = new Label();
methodVisitor.visitJumpInsn(IFEQ, label1);
Label label2 = new Label();
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(34, label2);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitFieldInsn(GETFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "target", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;");
methodVisitor.visitTypeInsn(NEW, "java/lang/StringBuffer");
methodVisitor.visitInsn(DUP);
methodVisitor.visitMethodInsn(INVOKESPECIAL, "java/lang/StringBuffer", "<init>", "()V", false);
methodVisitor.visitLdcInsn("NavigationHandler::updateGuidanceActive (ignored) - flag = ");
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "java/lang/StringBuffer", "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;", false);
methodVisitor.visitVarInsn(ILOAD, 1);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "java/lang/StringBuffer", "append", "(Z)Ljava/lang/StringBuffer;", false);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "java/lang/StringBuffer", "toString", "()Ljava/lang/String;", false);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget", "trace", "(Ljava/lang/String;)V", false);
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(44, label1);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(3, 2);
methodVisitor.visitEnd();
}
private static void member_8(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "updateServiceAvailable", "(Z)V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(48, label0);
methodVisitor.visitVarInsn(ILOAD, 1);
Label label1 = new Label();
methodVisitor.visitJumpInsn(IFEQ, label1);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitFieldInsn(GETFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "stopGuidancePending", "Z");
methodVisitor.visitJumpInsn(IFEQ, label1);
Label label2 = new Label();
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(49, label2);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitFieldInsn(PUTFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "stopGuidancePending", "Z");
Label label3 = new Label();
methodVisitor.visitLabel(label3);
methodVisitor.visitLineNumber(50, label3);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "stopGuidance", "()V", false);
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(52, label1);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(2, 2);
methodVisitor.visitEnd();
}
private static void member_9(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "stopGuidance", "()V", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(55, label0);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitFieldInsn(GETFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "target", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;");
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget", "isTraceEnabled", "()Z", false);
Label label1 = new Label();
methodVisitor.visitJumpInsn(IFEQ, label1);
Label label2 = new Label();
methodVisitor.visitLabel(label2);
methodVisitor.visitLineNumber(56, label2);
methodVisitor.visitVarInsn(ALOAD, 0);
methodVisitor.visitFieldInsn(GETFIELD, "de/vw/mib/asl/internal/androidauto/target/NavigationListener", "target", "Lde/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget;");
methodVisitor.visitLdcInsn("NavigationHandler::stopGuidance (ignored)");
methodVisitor.visitMethodInsn(INVOKEVIRTUAL, "de/vw/mib/asl/internal/androidauto/target/AndroidAutoTarget", "trace", "(Ljava/lang/String;)V", false);
methodVisitor.visitLabel(label1);
methodVisitor.visitLineNumber(66, label1);
methodVisitor.visitInsn(RETURN);
methodVisitor.visitMaxs(2, 1);
methodVisitor.visitEnd();
}
private static void member_10(ClassWriter classWriter) throws Exception {
FieldVisitor fieldVisitor;
RecordComponentVisitor recordComponentVisitor;
MethodVisitor methodVisitor;
AnnotationVisitor annotationVisitor0;
methodVisitor = classWriter.visitMethod(ACC_PUBLIC, "isGuidanceActive", "()Z", null, null);
methodVisitor.visitCode();
Label label0 = new Label();
methodVisitor.visitLabel(label0);
methodVisitor.visitLineNumber(70, label0);
methodVisitor.visitInsn(ICONST_0);
methodVisitor.visitInsn(IRETURN);
methodVisitor.visitMaxs(1, 1);
methodVisitor.visitEnd();
}
}

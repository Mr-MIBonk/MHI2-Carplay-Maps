import java.nio.file.*;
import java.util.*;
import java.util.zip.*;
import jdk.internal.org.objectweb.asm.*;

/** Offline exact-target experiment; retain the operand-stack result of fluent calls. */
public final class V53BapReturnAdapter implements Opcodes {
    public static void main(String[] args) throws Exception {
        final String owner="de/vw/mib/bap/datatypes/BAPString";
        final Set<String> targetMethods=new HashSet<String>();
        int changed=0;
        try(ZipFile stock=new ZipFile(args[1]); ZipFile input=new ZipFile(args[0]);
            ZipOutputStream out=new ZipOutputStream(Files.newOutputStream(Paths.get(args[2])))) {
            new ClassReader(stock.getInputStream(stock.getEntry(owner+".class")).readAllBytes()).accept(new ClassVisitor(ASM8) {
                public MethodVisitor visitMethod(int a,String n,String d,String s,String[] e) {
                    if(n.equals("setContent"))targetMethods.add(d);
                    return null;
                }
            },ClassReader.SKIP_CODE|ClassReader.SKIP_DEBUG|ClassReader.SKIP_FRAMES);
            for(ZipEntry entry:Collections.list(input.entries())) {
                byte[] bytes=input.getInputStream(entry).readAllBytes();
                if(entry.getName().endsWith(".class")) {
                    ClassReader reader=new ClassReader(bytes);
                    ClassWriter writer=new ClassWriter(reader,ClassWriter.COMPUTE_MAXS);
                    final int[] corrections={0};
                    reader.accept(new ClassVisitor(ASM8,writer) {
                        public MethodVisitor visitMethod(int a,String n,String d,String s,String[] e) {
                            return new MethodVisitor(ASM8,super.visitMethod(a,n,d,s,e)) {
                                public void visitMethodInsn(int op,String o,String method,String desc,boolean itf) {
                                    if(!o.equals(owner)||!method.equals("setContent")||targetMethods.contains(desc)) {
                                        super.visitMethodInsn(op,o,method,desc,itf);return;
                                    }
                                    String parameters=desc.substring(0,desc.indexOf(')')+1);
                                    String fluent=parameters+"L"+owner+";", plain=parameters+"V";
                                    if(desc.equals(plain)&&targetMethods.contains(fluent)) {
                                        super.visitMethodInsn(op,o,method,fluent,itf);
                                        super.visitInsn(POP);
                                    } else if(desc.equals(fluent)&&targetMethods.contains(plain)) {
                                        Type[] arguments=Type.getArgumentTypes(desc);
                                        if(op!=INVOKEVIRTUAL||arguments.length!=1||arguments[0].getSize()!=1)
                                            throw new IllegalStateException("Unsupported BAPString stack form "+desc);
                                        // obj,arg -> obj,arg,obj,arg -> obj,arg -> obj.
                                        // A fluent method returns its receiver; preserve that result even
                                        // when the exact target implementation has a void return type.
                                        super.visitInsn(DUP2);
                                        super.visitMethodInsn(op,o,method,plain,itf);
                                        super.visitInsn(POP);
                                    } else throw new IllegalStateException("No target BAPString method "+desc);
                                    corrections[0]++;
                                }
                            };
                        }
                    },ClassReader.SKIP_FRAMES);
                    if(corrections[0]>0){bytes=writer.toByteArray();changed+=corrections[0];}
                }
                ZipEntry copy=new ZipEntry(entry.getName());copy.setTime(315532802000L);
                out.putNextEntry(copy);out.write(bytes);out.closeEntry();
            }
        }
        System.out.println("BAP_RETURN_ADAPTER_PASS corrected_calls="+changed);
    }
}

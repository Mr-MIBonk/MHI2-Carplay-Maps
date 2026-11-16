import java.nio.file.*;
import jdk.internal.org.objectweb.asm.*;

/** omonob: remove owned welcome-page hooks, retain the native lifecycle. */
public final class StripSoundHooks {
    public static void main(String[] args)throws Exception {
        byte[] input=Files.readAllBytes(Path.of(args[0]));
        ClassWriter output=new ClassWriter(ClassWriter.COMPUTE_MAXS);
        final int[] removed={0};
        new ClassReader(input).accept(new ClassVisitor(Opcodes.ASM8,output){
            public MethodVisitor visitMethod(int access,String name,String desc,String sig,String[] exceptions){
                return new MethodVisitor(Opcodes.ASM8,super.visitMethod(access,name,desc,sig,exceptions)){
                    public void visitMethodInsn(int opcode,String owner,String method,String descriptor,boolean itf){
                        if(owner.equals("org/omonob/cpmap/ui/SoundMenuBridge")){
                            if(opcode!=Opcodes.INVOKESTATIC||Type.getReturnType(descriptor).getSort()!=Type.VOID)
                                throw new IllegalStateException("Unknown sound hook ABI");
                            Type[] parameters=Type.getArgumentTypes(descriptor);
                            for(int i=parameters.length-1;i>=0;i--)
                                super.visitInsn(parameters[i].getSize()==2?Opcodes.POP2:Opcodes.POP);
                            removed[0]++;
                        }else super.visitMethodInsn(opcode,owner,method,descriptor,itf);
                    }
                };
            }
        },ClassReader.SKIP_FRAMES);
        if(removed[0]!=4)throw new IllegalStateException("Expected four R26 sound hooks: "+removed[0]);
        byte[] result=output.toByteArray();
        if(new String(result,java.nio.charset.StandardCharsets.ISO_8859_1).contains("SoundMenuBridge"))
            throw new IllegalStateException("Unremoved sound reference");
        Files.write(Path.of(args[1]),result);
        System.out.println("OMONOB_SOUND_HOOK_REMOVAL_PASS hooks="+removed[0]);
    }
}

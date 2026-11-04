import java.io.*;
import java.util.*;
import java.util.jar.*;
import jdk.internal.org.objectweb.asm.*;
import jdk.internal.org.objectweb.asm.util.*;
/** Optional provenance check against the author-supplied historical patch. */
public final class VerifyHistoricalSource {
    static String trace(byte[] bytes) {
        StringWriter result=new StringWriter();
        new ClassReader(bytes).accept(new TraceClassVisitor(new PrintWriter(result)),ClassReader.SKIP_DEBUG|ClassReader.SKIP_FRAMES);
        return result.toString();
    }
    public static void main(String[] args)throws Exception {
        int count=0;
        try(JarFile original=new JarFile(args[0]);JarFile assembled=new JarFile(args[1])) {
            for(JarEntry entry:Collections.list(original.entries())) {
                if(!entry.getName().endsWith(".class"))continue;
                JarEntry copy=assembled.getJarEntry(entry.getName());
                if(copy==null||!trace(original.getInputStream(entry).readAllBytes()).equals(
                    trace(assembled.getInputStream(copy).readAllBytes())))
                    throw new AssertionError("Historical instruction source mismatch: "+entry.getName());
                count++;
            }
        }
        if(count!=49)throw new AssertionError("Historical class count mismatch");
        System.out.println("HISTORICAL_INSTRUCTION_EQUIVALENCE_PASS classes="+count+" debug_and_stackmap_only_ignored=1");
    }
}

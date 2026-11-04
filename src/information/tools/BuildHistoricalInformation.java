import java.nio.file.*;
import java.util.*;
import java.util.jar.*;

/** Build the historical V53 patch from instruction source, without a donor JAR. */
public final class BuildHistoricalInformation {
    public static void main(String[] args)throws Exception {
        Path index=Path.of(args[0]), output=Path.of(args[1]);
        Files.createDirectories(output.getParent());int count=0;
        try(JarOutputStream jar=new JarOutputStream(Files.newOutputStream(output))) {
            for(String row:Files.readAllLines(index)) {
                if(row.isBlank())continue;
                String[] entry=row.split("\t");
                if(entry.length!=2||entry[1].contains("..")||!entry[1].endsWith(".class"))
                    throw new IllegalStateException("Invalid instruction source index");
                byte[] bytes=(byte[])Class.forName(entry[0]).getMethod("dump").invoke(null);
                JarEntry name=new JarEntry(entry[1]);name.setTime(315532802000L);
                jar.putNextEntry(name);jar.write(bytes);jar.closeEntry();count++;
            }
        }
        if(count!=49)throw new IllegalStateException("Historical patch class count changed: "+count);
        System.out.println("HISTORICAL_INFORMATION_SOURCE_BUILD_PASS classes="+count+" donor_binary_required=0");
    }
}

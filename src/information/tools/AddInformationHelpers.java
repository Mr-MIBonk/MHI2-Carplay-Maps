import java.nio.file.*;
import java.util.*;
import java.util.jar.*;
/** Add only this project's compiled, Java-1.2-normalized dispatch helper. */
public final class AddInformationHelpers {
 public static void main(String[] a)throws Exception {
  Path root=Path.of(a[1]);int added=0;
  try(JarFile source=new JarFile(a[0]);JarOutputStream out=new JarOutputStream(Files.newOutputStream(Path.of(a[2])))) {
   for(JarEntry e:Collections.list(source.entries())) {
    JarEntry copy=new JarEntry(e.getName());copy.setTime(0);out.putNextEntry(copy);out.write(source.getInputStream(e).readAllBytes());out.closeEntry();
   }
   List<Path> files=new ArrayList<Path>();try(java.util.stream.Stream<Path> walk=Files.walk(root)){walk.filter(p->p.toString().endsWith(".class")).forEach(files::add);}
   Collections.sort(files);
   for(Path file:files){String name=root.relativize(file).toString().replace('\\','/');
    if(!name.startsWith("org/omonob/cpmap/information/InformationAslDispatch"))throw new IllegalStateException(name);
    JarEntry e=new JarEntry(name);e.setTime(0);out.putNextEntry(e);out.write(BuildNativeHook.legacy(Files.readAllBytes(file)));out.closeEntry();added++;
   }
  }
  if(added!=2)throw new IllegalStateException("Dispatch helper class count "+added);
  System.out.println("INFORMATION_DISPATCH_HELPER_ADDED classes="+added);
 }
}

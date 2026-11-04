import java.io.*;
import java.net.*;
import java.util.*;
import java.util.zip.*;
public class LinkSmoke {
  public static void main(String[] args)throws Exception {
    int passed=0;List<String> failures=new ArrayList<>();
    try(URLClassLoader loader=new URLClassLoader(new URL[]{new File(args[0]).toURI().toURL(),new File(args[1]).toURI().toURL()},ClassLoader.getPlatformClassLoader());ZipFile jar=new ZipFile(args[0])){
      for(ZipEntry e:Collections.list(jar.entries())){
        if(!e.getName().endsWith(".class"))continue;
        String name=e.getName().substring(0,e.getName().length()-6).replace('/','.');
        try{Class<?> c=Class.forName(name,false,loader);c.getDeclaredFields();c.getDeclaredMethods();c.getDeclaredConstructors();passed++;}
        catch(Throwable t){failures.add(name+": "+t);}
      }
    }
    System.out.println("JVM verification: passed="+passed+" failed="+failures.size());for(String f:failures)System.out.println(f);
    if(!failures.isEmpty())System.exit(1);
  }
}

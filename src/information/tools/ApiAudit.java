import java.nio.file.*;
import java.util.*;
import java.util.zip.*;
import jdk.internal.org.objectweb.asm.*;

public class ApiAudit implements Opcodes {
  static class Info {
    String name, parent; String[] interfaces;
    Map<String,Integer> methods=new TreeMap<>(), fields=new TreeMap<>();
  }
  static class Repo implements AutoCloseable {
    ZipFile stock,patch; Map<String,Info> cache=new HashMap<>();
    Repo(String s,String p) throws Exception {stock=new ZipFile(s); if(p!=null)patch=new ZipFile(p);}
    Info info(String name) {
      if(cache.containsKey(name))return cache.get(name);
      try {
        ZipEntry e=patch==null?null:patch.getEntry(name+".class"); ZipFile z=patch;
        if(e==null){e=stock.getEntry(name+".class");z=stock;}
        if(e==null){
          if(name.startsWith("java/")||name.startsWith("javax/")){
            try(java.io.InputStream in=ClassLoader.getSystemResourceAsStream(name+".class")){
              if(in!=null){Info i=parse(in.readAllBytes());cache.put(name,i);return i;}
            }
          }
          cache.put(name,null);return null;
        }
        Info i=parse(z.getInputStream(e).readAllBytes());cache.put(name,i);return i;
      }catch(Exception e){throw new RuntimeException(e);}
    }
    Integer resolve(String owner,String key,boolean method,Set<String> seen) {
      if(!seen.add(owner))return null;
      Info i=info(owner);if(i==null)return null;
      Integer a=(method?i.methods:i.fields).get(key);if(a!=null)return a;
      if(method&&key.startsWith("<init>"))return null;
      if(i.parent!=null){a=resolve(i.parent,key,method,seen);if(a!=null)return a;}
      for(String p:i.interfaces){a=resolve(p,key,method,seen);if(a!=null)return a;}
      return null;
    }
    Integer resolve(String owner,String key,boolean method){return resolve(owner,key,method,new HashSet<>());}
    public void close() throws Exception {stock.close();if(patch!=null)patch.close();}
  }
  static Info parse(byte[] b) {
    Info i=new Info();
    new ClassReader(b).accept(new ClassVisitor(ASM8){
      public void visit(int v,int a,String n,String s,String p,String[] it){i.name=n;i.parent=p;i.interfaces=it;}
      public FieldVisitor visitField(int a,String n,String d,String s,Object v){i.fields.put(n+d,a);return null;}
      public MethodVisitor visitMethod(int a,String n,String d,String s,String[] e){i.methods.put(n+d,a);return null;}
    },ClassReader.SKIP_CODE|ClassReader.SKIP_DEBUG|ClassReader.SKIP_FRAMES);return i;
  }
  public static void main(String[] args)throws Exception {
    // source stock, target stock, payload, output report
    List<String> out=new ArrayList<>();Set<String> refs=new TreeSet<>();int overrides=0,classes=0;
    try(ZipFile payload=new ZipFile(args[2]);Repo source=new Repo(args[0],null);Repo target=new Repo(args[1],args[2]);Repo targetStock=new Repo(args[1],null)) {
      for(ZipEntry e:Collections.list(payload.entries())){
        if(!e.getName().endsWith(".class"))continue;classes++;
        byte[] bytes=payload.getInputStream(e).readAllBytes();Info patched=parse(bytes),old=source.info(patched.name),stock=targetStock.info(patched.name);
        if(old!=null){
          overrides++;
          if(stock==null)out.add("MISSING_CLASS\t"+patched.name);
          else{
            if(!Objects.equals(old.parent,stock.parent)||!Arrays.equals(old.interfaces,stock.interfaces))out.add("PARENTS\t"+patched.name+"\t"+old.parent+" -> "+stock.parent+"\t"+Arrays.toString(old.interfaces)+" -> "+Arrays.toString(stock.interfaces));
            for(Map.Entry<String,Integer> m:stock.methods.entrySet())if((m.getValue()&ACC_PRIVATE)==0 && !patched.methods.containsKey(m.getKey()) && target.resolve(patched.name,m.getKey(),true)==null)out.add("TARGET_API_MISSING_IN_PATCH\t"+patched.name+"\t"+m.getKey());
            for(Map.Entry<String,Integer> m:stock.fields.entrySet())if((m.getValue()&ACC_PRIVATE)==0 && !patched.fields.containsKey(m.getKey()) && target.resolve(patched.name,m.getKey(),false)==null)out.add("TARGET_FIELD_MISSING_IN_PATCH\t"+patched.name+"\t"+m.getKey());
            for(String key:old.methods.keySet())if((old.methods.get(key)&ACC_PRIVATE)==0 && !stock.methods.containsKey(key))out.add("STOCK_METHOD_DIFF\t"+patched.name+"\t"+key);
            for(String key:old.fields.keySet())if((old.fields.get(key)&ACC_PRIVATE)==0 && !stock.fields.containsKey(key))out.add("STOCK_FIELD_DIFF\t"+patched.name+"\t"+key);
          }
        }
        new ClassReader(bytes).accept(new ClassVisitor(ASM8){
          void type(String t){
            if(t==null)return;
            if(t.startsWith("[")){descriptor(t);return;}
            if(!t.startsWith("java/")&&!t.startsWith("javax/")&&target.info(t)==null)refs.add("MISSING_TYPE\t"+patched.name+"\t"+t);
          }
          void descriptor(String d){
            if(d==null)return;
            java.util.regex.Matcher m=java.util.regex.Pattern.compile("L([^;]+);").matcher(d);
            while(m.find())type(m.group(1));
          }
          public void visit(int v,int a,String n,String s,String p,String[] it){type(p);for(String x:it)type(x);}
          public FieldVisitor visitField(int a,String n,String d,String s,Object value){descriptor(d);return null;}
          public MethodVisitor visitMethod(int a,String n,String d,String s,String[] ex){descriptor(d);if(ex!=null)for(String x:ex)type(x);return new MethodVisitor(ASM8){
            void check(String owner,String name,String desc,boolean method,int op){
              if(owner.startsWith("java/")||owner.startsWith("javax/")||owner.startsWith("["))return;
              Integer access=target.resolve(owner,name+desc,method);
              if(access==null)refs.add("UNRESOLVED\t"+patched.name+"."+n+"\t"+owner+"\t"+name+desc);
              else if(((access&ACC_STATIC)!=0)!=(op==INVOKESTATIC||op==GETSTATIC||op==PUTSTATIC))refs.add("STATIC_MISMATCH\t"+patched.name+"."+n+"\t"+owner+"\t"+name+desc);
            }
            public void visitMethodInsn(int op,String o,String n,String d,boolean it){type(o);descriptor(d);check(o,n,d,true,op);}
            public void visitFieldInsn(int op,String o,String n,String d){type(o);descriptor(d);check(o,n,d,false,op);}
            public void visitTypeInsn(int op,String t){type(t);}
            public void visitLdcInsn(Object value){if(value instanceof Type)descriptor(((Type)value).getDescriptor());}
          };}
        },ClassReader.SKIP_DEBUG|ClassReader.SKIP_FRAMES);
      }
    }
    out.addAll(refs);Files.write(Paths.get(args[3]),out);
    Map<String,Integer> counts=new TreeMap<>();for(String l:out)counts.merge(l.split("\t")[0],1,Integer::sum);
    System.out.println("classes="+classes+" overrides="+overrides+" counts="+counts);
    for(String l:out)if(!l.startsWith("UNRESOLVED"))System.out.println(l);
    System.out.println("report="+args[3]);
  }
}

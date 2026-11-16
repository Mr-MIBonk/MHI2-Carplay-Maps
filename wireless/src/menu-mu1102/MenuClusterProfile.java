package org.omonob.cpmap.ui;
import java.io.FileInputStream;
import java.io.File;
/** Exact installer profile; never infer a cluster from the phone canvas. */
public final class MenuClusterProfile {
 private MenuClusterProfile(){}
 public static int cluster(){
  String test=System.getProperty("cpmap.ui.cluster");
  if(test!=null && "1".equals(System.getProperty("cpvc.offline"))){
   if("790".equals(test))return 790;if("791".equals(test))return 791;
  }
  String profile=readOptional("/mnt/app/carplaymaps/PROFILE");
  return selectProfile(profile,profile==null?readOptional("/mnt/app/carplaymaps/VERSION"):null,
                       readOptional("/mnt/persist/omonob-phone/cluster-profile"));
 }
 public static int selectProfile(String profile,String version,String confirmed){
  if(profile!=null)return select(profile,confirmed);
  // Only historical device-prefixed values carry a hardware identity.
  // Current product release names must never be parsed as cluster models.
  if(version!=null){String text=version.trim();
   if(text.startsWith("3GD920790-")||text.startsWith("3G0920791-"))return select(text,confirmed);
  }
  return select(null,confirmed);
 }
 public static int select(String installed,String confirmed){
  int known=0,chosen=0;
  if(installed!=null){String text=installed.trim();
   if(text.equals("MU1102/790")||text.equals("MU1367/790")||text.equals("MU1440/790")||text.equals("MU1433/790")||text.equals("MU1447/790")||text.startsWith("3GD920790-"))known=790;
   else if(text.equals("MU1102/791")||text.equals("MU1367/791")||text.startsWith("3G0920791-"))known=791;
   else throw new IllegalStateException("Unknown installed cluster profile");
  }
  if(confirmed!=null){String text=confirmed.trim();
   if("3GD920790".equals(text))chosen=790;
   else if("3G0920791".equals(text))chosen=791;
   else throw new IllegalStateException("Unknown confirmed cluster profile");
  }
  if(known!=0&&chosen!=0&&known!=chosen)throw new IllegalStateException("Conflicting cluster profiles");
  if(known!=0)return known;if(chosen!=0)return chosen;
  throw new IllegalStateException("No confirmed cluster profile");
 }
 private static String readOptional(String path){
  File file=new File(path);if(!file.exists())return null;
  FileInputStream in=null;
  try{
   if(!file.isFile()||file.length()<1||file.length()>256)throw new IllegalStateException("Invalid cluster profile length");
   byte[] bytes=new byte[(int)file.length()];in=new FileInputStream(file);int n=0;
   while(n<bytes.length){int count=in.read(bytes,n,bytes.length-n);if(count<0)throw new IllegalStateException("Incomplete cluster profile");n+=count;}
   if(in.read()!=-1)throw new IllegalStateException("Changed cluster profile");
   return new String(bytes,"US-ASCII");
  }catch(Exception error){throw new IllegalStateException(error.toString());}
  finally{if(in!=null)try{in.close();}catch(Exception ignored){}}
 }
 public static boolean is791(){return cluster()==791;}
 public static int cardChoice(int position){return is791()?Card791Options.choiceForPosition(position):position;}
 public static int cardPosition(int choice){return is791()?Card791Options.positionForChoice(choice):choice;}
 public static int cardCount(){return is791()?2:4;}
 public static String cardLabel(String language,int choice){return is791()?Card791Options.label(language,choice):CardMenuMessages.get(language,choice+1);}
}

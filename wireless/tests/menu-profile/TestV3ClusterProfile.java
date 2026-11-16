import org.omonob.cpmap.ui.MenuClusterProfile;
public final class TestV3ClusterProfile {
 private static int checks;
 private static void equal(int got,int wanted){if(got!=wanted)throw new AssertionError("profile mismatch");checks++;}
 private static void rejects(String profile,String version,String confirmed){
  try{MenuClusterProfile.selectProfile(profile,version,confirmed);}catch(IllegalStateException expected){checks++;return;}
  throw new AssertionError("unsafe profile accepted");
 }
 public static void main(String[] args){
  String[] releases={"WIRELESS-CARPLAY-MAPS-LOCAL-1","WIRELESS-CARPLAY-MAPS-LOCAL-2","WIRELESS-CARPLAY-MAPS-3.0","another-product-release"};
  String[] profiles={"MU1102/790","MU1367/790","MU1440/790","MU1433/790","MU1447/790","MU1102/791","MU1367/791"};
  for(int i=0;i<profiles.length;i++)for(int j=0;j<releases.length;j++){
   int wanted=profiles[i].endsWith("791")?791:790;
   equal(MenuClusterProfile.selectProfile(profiles[i],releases[j],null),wanted);
   equal(MenuClusterProfile.selectProfile(profiles[i],releases[j],wanted==790?"3GD920790":"3G0920791"),wanted);
   rejects(profiles[i],releases[j],wanted==790?"3G0920791":"3GD920790");
  }
  equal(MenuClusterProfile.selectProfile(null,"WIRELESS-CARPLAY-MAPS-3.0","3GD920790"),790);
  equal(MenuClusterProfile.selectProfile(null,"WIRELESS-CARPLAY-MAPS-3.0","3G0920791"),791);
  equal(MenuClusterProfile.selectProfile(null,"3GD920790-legacy",null),790);
  equal(MenuClusterProfile.selectProfile(null,"3G0920791-legacy",null),791);
  rejects(null,"WIRELESS-CARPLAY-MAPS-3.0",null);
  rejects("MU1102/unknown","WIRELESS-CARPLAY-MAPS-3.0","3GD920790");
  rejects("unknown","WIRELESS-CARPLAY-MAPS-3.0",null);
  rejects(null,"3GD920790-legacy","3G0920791");
  System.out.println("V3_CLUSTER_PROFILE_PASS checks="+checks);
 }
}

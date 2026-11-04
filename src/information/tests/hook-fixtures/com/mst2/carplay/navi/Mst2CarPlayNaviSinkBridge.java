package com.mst2.carplay.navi;
/** Service-free original-body fixture, patched with the real injection tool. */
public final class Mst2CarPlayNaviSinkBridge {
 public static int clears,resumes;
 public static boolean shouldWriteNow(){return true;}
 public static boolean isOverrideActive(){return true;}
 private static void sendCleanupVolley(){clears++;}
 private static void triggerBapShadows(String reason){resumes++;}
}

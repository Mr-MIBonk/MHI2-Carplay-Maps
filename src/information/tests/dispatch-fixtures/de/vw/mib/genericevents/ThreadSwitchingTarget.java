package de.vw.mib.genericevents;
/** Service-free fixture, not a replacement for the vehicle interface. */
public interface ThreadSwitchingTarget {void enqueue(Runnable job);boolean isThreadChangeNecessary();}

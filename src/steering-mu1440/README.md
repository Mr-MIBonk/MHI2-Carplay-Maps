# MU1440 Skoda roller control

This is the roller-control implementation used for the MU1440 790 profile in toolbox v1.1.0. Existing Volkswagen button behavior and native display payloads are unchanged. It passively observes `updateEncoder2(4, 40, +1/-1, 0, 1)` and sends `zoom-in` or `zoom-out` to the existing loopback control port 7032. Roller presses and other encoder IDs are ignored. Original DSI listeners still receive the events.

Build with JDK 8:

```powershell
.\build.ps1 -OutputJar C:\Build\CarPlayClusterControls.jar
```

The compiler targets Java 1.4, classfile major 48. Compilation stubs are excluded from the runtime JAR. `TestSkodaRollerBridge` checks valid upward/downward detents and rejection of unverified input. On the vehicle, a phone map app must support zoom commands.

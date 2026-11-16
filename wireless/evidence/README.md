# Evidence scope

The R2 reports `r2-install-test.json`, `r2-public-upgrades.json`, `r2-clean-source-build.json`, and `r2-audit.json` describe this revision. The historical test inputs are identified in `public-history-inputs.json`; their complete old packages are deliberately not redistributed here.

Other reports were copied unchanged from the R1 preparation record. They document earlier builds, not the active selection list. In particular, old boot/receiver reports can mention the now-retired second phone profile. R2 has only iPhone 17 Pro Max, with distinct hardware profiles for the two supported clusters. The old script filename is a forwarding shim needed by the R1 updater, not an active display declaration.

All current R2 installation tests use synthetic host filesystems. They do not prove a real QNX loader, GPU, phone, radio or touchscreen has accepted this new lean combination. The native source rebuild is not a complete rebuild of the opaque receiver or OEM/SDK components, and its artifacts are not automatically substituted into the delivered frozen payload.

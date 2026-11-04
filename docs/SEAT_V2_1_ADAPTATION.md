# SEAT MU1447 V2.1 experimental adaptation

This document records source changes and their provenance for the V2.1
experimental prerelease. It is not a claim of independent physical-vehicle
acceptance. See [the validation report](V2_1_VALIDATION.md).

## Contributors

**Special thanks to [olli991](https://github.com/olli991)** for extensive
development ideas, practical suggestions and SEAT testing feedback.
Thanks to [octacon84](https://github.com/octacon84) for submitting the MU1447
adaptation package and sharing community testing results.
See [the bilingual acknowledgements](COMMUNITY_V2_1_CREDITS.md).

## Exact target

MHI2_ER_SEG11_P4709 / MU1447, AID 10.5, 1010×376 instrument output.
The head-unit's native G11 menu is separately 800×480. No MU1409 binary is
installed, and this change does not expand the exact firmware whitelist.

## Community input audit

Submitted `MU1447.zip` SHA-256:
`9b85ae292c0e62a5843433f6becfe30347f05b72c2d672d26245aa0fddb52dfc`.

The submitted VCMenu.jar and VCMirrorRate.jar are byte-identical to the
previous public MU1447 build. CarPlayClusterControls.jar is byte-identical to
the public MU1440 roller build. Its source can therefore be rebuilt using the
existing verified roller baseline rather than importing an unknown dispatcher.
The submitted native core differs from the current public native core.

The following four SafeArea rectangles were read from the submitted core's
ELF symbol table and reconstructed in source. This is not a change in output
resolution, decoder format, or a software crop of encoded video.

| View | x | y | width | height |
| --- | ---: | ---: | ---: | ---: |
| Idle | 202 | 1 | 606 | 329 |
| Left scene | 202 | 45 | 606 | 272 |
| Right scene | 202 | 45 | 606 | 272 |
| Compact center scene | 325 | 40 | 360 | 260 |

Left/right scene URLs remain different even though the SEAT safe rectangles
match. The fifth view, used by our newer pure-map navigation-ownership path,
reuses SEAT's idle rectangle. This fifth-view choice is our integration decision,
not a value claimed to have been tested in the four-view contribution.
Volkswagen's x=42 pure-map correction is not applied to SEAT.

## Remaining validation boundaries

olli991's observations about day/night appearance and settings needing to be
reapplied after a reboot remain separate issues to reproduce. A successful
command callback is not proof of correct visible rendering. Do not call either
issue fixed without the appropriate session/vehicle evidence.

The separately credited
[MU1409 reference](https://github.com/RandomGenerated543/seat-cupra-mu1409-carplay)
describes another exact firmware and different calibration candidates. It is
useful research, not a drop-in replacement for MU1447.

# fx9860-large-loader

External Flash execution backends for fx-9860GIII, including the experimental
path beyond the OS's 512-page / 2 MiB mapping window.

Hardware first launches were reported working for KhiCAS and integrated Giac,
but same-icon MENU return failed. The Giac RC2 launcher adds an explicit reload
page; original KhiCAS resume remains unresolved on hardware. See
[the findings and retest procedure](HARDWARE-20261117.md).

* `probes/mapped-loader`: exact-version original KhiCAS adapter; 512-page OS
  mapping, four shadow pages, resident syscall/callback maintenance. Full
  calculator candidate exists; complete hardware acceptance is pending.
* `probes/extended-loader`: independent 698-page guest-refill execution probe.
  Synthetic code/data passed simulation; full CAS integration and hardware
  acceptance are separate. This is not a universal oversized-G1A converter.
* `probes/extended-cas`: source-integrated Giac RC2 loader, 699-page payload,
  39,680-byte combined resident/arena allocation. The full CAS candidate passed
  simulation, including calculation, plotting and cleanup; hardware acceptance
  is pending. Payload source adapters are included; the math archive is external.

Open either directory in the FxStudio offline SDK and build Debug, then Release.
The source retains authored relocation checks. Use the unified PC tool in
[fx9860-loader](https://github.com/lilith-is-all-you-need/fx9860-loader)
to classify inputs, generate profiles and prepare exact-version bundles.

See the [mapped instructions](probes/mapped-loader/README.zh-CN.md) and
[extended runtime contract](probes/extended-loader/README.md). Tests requiring an
emulator use the separately built [fx9860-emulator](https://github.com/lilith-is-all-you-need/fx9860-emulator).
Firmware, original KhiCAS/Giac binaries and private traces are not included.

`probes/native-loader/tools/prepare.py` is a small synchronized scanner snapshot
from fx9860-loader, retained so offline mapped profile generation is standalone.
Code is MIT; third-party program licenses remain their owners'.

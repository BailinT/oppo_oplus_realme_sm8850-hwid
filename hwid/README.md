# HWID allowlist

All non-comment entries in `allowlist.txt` are compiled into the same kernel
image. A device may boot only when its HWID is present in that file.

The kernel probes all known boot parameter keys and checks each value found
against the allowlist:

- `androidboot.chipid`    (OPPO / OnePlus / Realme)
- `androidboot.cpuid`     (Xiaomi / Redmi / POCO / Black Shark)
- `androidboot.emmcid`    (Nubia / Red Magic)
- `androidboot.serialno`  (vivo / iQOO / Lenovo / Motorola / fallback)
- `oplusboot.serialno`    (OPPO alternate)

Android bootconfig and the legacy kernel command line are both supported.
Hex values are compared numerically, so `0x0000046a10dfd755`,
`0000046a10dfd755` and `46a10dfd755` all match. A missing or unauthorized
value causes a kernel panic and an automatic reboot after one second, before
Android userspace starts.

Add one HWID per line and rebuild the kernel after changing the list.

## Dry run

Set the workflow env `HWID_LOCK_DRY_RUN=1` or create `hwid/DRYRUN` to
build a non-locking kernel that logs every key it finds. Run
`dmesg | grep "HWID lock"` on the device, confirm the value, then turn
dry run off and rebuild. Never ship a dry-run build as the locked build.

## Probe script

Push `probe_hwid.sh` to a working kernel and run it to dump every boot
parameter that might carry a device ID. Pass the expected ID as the first
argument to get match markers.

Keep a known-good boot image and a working fastboot/recovery rollback path
before flashing a test build.

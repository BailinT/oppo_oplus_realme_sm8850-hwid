# HWID allowlist

All non-comment entries in `allowlist.txt` are compiled into the same kernel
image. A device may boot only when its HWID is present in that file.

The kernel reads `androidboot.chipid` first and falls back to
`oplusboot.serialno`. Android bootconfig and the legacy kernel command line
are both supported. A missing or unauthorized value causes a kernel panic and
an automatic reboot after one second, before Android userspace starts.

Add one HWID per line and rebuild the kernel after changing the list. The
initial allowlist contains `abbe0d2c`.

Keep a known-good boot image and a working fastboot/recovery rollback path
before flashing a test build.

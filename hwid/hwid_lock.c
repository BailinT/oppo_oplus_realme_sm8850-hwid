// SPDX-License-Identifier: GPL-2.0-only
#include <linux/bootconfig.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include "hwid_allowlist.h"

extern char boot_command_line[];

/*
 * Boot parameters that may carry a per-device unique id. Different vendors
 * expose it under different names, so every key is probed and every value
 * found is checked against the allowlist. Do not stop at the first key that
 * exists: a device can expose several of these at once and only one of them
 * is the id the user actually put in the allowlist.
 */
static const char *const hwid_keys[] __initconst = {
	"androidboot.chipid",
	"androidboot.cpuid",
	"androidboot.emmcid",
	"androidboot.serialno",
	"oplusboot.serialno",
};

static const char *__init hwid_from_cmdline(const char *key, char *value,
					     size_t value_size)
{
	const char *cursor = boot_command_line;
	size_t key_len = strlen(key);

	while ((cursor = strstr(cursor, key)) != NULL) {
		const char *start;
		size_t len;

		if (cursor != boot_command_line && cursor[-1] != ' ') {
			cursor += key_len;
			continue;
		}

		start = cursor + key_len;
		if (*start != '=') {
			cursor += key_len;
			continue;
		}

		start++;
		if (*start == '"')
			start++;
		len = strcspn(start, " \"");
		if (!len || len >= value_size)
			return NULL;

		memcpy(value, start, len);
		value[len] = '\0';
		return value;
	}

	return NULL;
}

/*
 * Compare an id reported by the bootloader with an allowlist entry. Exact
 * (case-insensitive) match first; if both sides parse as hex, compare
 * numerically so that "0x0000044864acd92f", "0000044864acd92f" and
 * "44864acd92f" are treated as the same device.
 */
static bool __init hwid_match(const char *detected, const char *allowed)
{
	unsigned long long detected_val, allowed_val;

	if (!strcasecmp(detected, allowed))
		return true;

	if (!kstrtoull(detected, 16, &detected_val) &&
	    !kstrtoull(allowed, 16, &allowed_val))
		return detected_val == allowed_val;

	return false;
}

static bool __init hwid_allowed(const char *hwid)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hwid_allowlist); i++) {
		if (hwid_match(hwid, hwid_allowlist[i]))
			return true;
	}

	return false;
}

static const char *__init hwid_read_key(const char *key, char *value,
					 size_t value_size)
{
#ifdef CONFIG_BOOT_CONFIG
	struct xbc_node *value_node;
	const char *bootconfig_value = xbc_find_value(key, &value_node);

	if (bootconfig_value && *bootconfig_value) {
		strscpy(value, bootconfig_value, value_size);
		return value;
	}
#endif
	return hwid_from_cmdline(key, value, value_size);
}

void __init hwid_lock_verify(void)
{
	char value[HWID_LOCK_MAX_ID_LEN];
	bool found_any = false;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hwid_keys); i++) {
		const char *detected = hwid_read_key(hwid_keys[i], value,
						     sizeof(value));

		if (!detected)
			continue;

		found_any = true;
		pr_info("HWID lock: found %s=%s\n", hwid_keys[i], detected);

		if (hwid_allowed(detected)) {
			pr_info("HWID lock: device authorized via %s\n",
				hwid_keys[i]);
			return;
		}
	}

#ifdef HWID_LOCK_DRY_RUN
	pr_warn("HWID lock: DRY RUN - %s, boot continues\n",
		found_any ? "no allowlist match" :
			    "no HWID boot parameter found");
#else
	panic_timeout = 1;
	if (!found_any)
		panic("HWID lock: no HWID boot parameter found");
	panic("HWID lock: this kernel is not authorized for this device");
#endif
}

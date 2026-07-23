// SPDX-License-Identifier: GPL-2.0-only
#include <linux/bootconfig.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/panic.h>
#include <linux/string.h>

#include "hwid_allowlist.h"

extern char boot_command_line[];

static const char *const hwid_keys[] __initconst = {
	"androidboot.chipid",
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

static const char *__init hwid_read(char *value, size_t value_size)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hwid_keys); i++) {
#ifdef CONFIG_BOOT_CONFIG
		struct xbc_node *value_node;
		const char *bootconfig_value = xbc_find_value(hwid_keys[i],
							       &value_node);

		if (bootconfig_value && *bootconfig_value) {
			strscpy(value, bootconfig_value, value_size);
			return value;
		}
#endif
		if (hwid_from_cmdline(hwid_keys[i], value, value_size))
			return value;
	}

	return NULL;
}

static bool __init hwid_allowed(const char *hwid)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hwid_allowlist); i++) {
		if (!strcasecmp(hwid, hwid_allowlist[i]))
			return true;
	}

	return false;
}

void __init hwid_lock_verify(void)
{
	char hwid[HWID_LOCK_MAX_ID_LEN];
	const char *detected = hwid_read(hwid, sizeof(hwid));

	if (!detected) {
		panic_timeout = 1;
		panic("HWID lock: androidboot.chipid/oplusboot.serialno is missing");
	}

	if (!hwid_allowed(detected)) {
		panic_timeout = 1;
		panic("HWID lock: this kernel is not authorized for this device");
	}

	pr_info("HWID lock: device authorized\n");
}

/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/os.h>
#include <xt/error.h>

#include <ctype.h>
#include <string.h>

int _xtConsoleFillLine(const char *pattern)
{
	// Only initializing it because otherwise GCC warns
	// us if we use the -O3 option
	unsigned width = 0;
	if (!pattern)
		return XT_EINVAL;
	int ret = xtConsoleGetSize(&width, NULL);
	if (ret)
		return ret;
	const char *str = pattern;
	while (width-- > 0) {
		if (!*str)
			str = pattern;
		putchar(*str++);
	}
	return 0;
}

bool xtBatteryIsPresent(void)
{
	return xtBatteryGetPowerLevel() != -1;
}

void xtCPUDump(const struct xtCPUInfo *restrict cpuInfo, FILE *restrict f)
{
	fprintf(f, "CPU name: %s\n", cpuInfo->name);
	char cpuArch[16];
	switch (cpuInfo->architecture) {
	case XT_CPU_ARCH_X64: strncpy(cpuArch, "x64", sizeof cpuArch); break;
	case XT_CPU_ARCH_X86: strncpy(cpuArch, "x86", sizeof cpuArch); break;
	case XT_CPU_ARCH_ARM: strncpy(cpuArch, "ARM", sizeof cpuArch); break;
	case XT_CPU_ARCH_IA64: strncpy(cpuArch, "IA64", sizeof cpuArch); break;
	default: strncpy(cpuArch, "Unknown", sizeof cpuArch); break;
	}
	fprintf(f, "CPU architecture: %s\n", cpuArch);
	fprintf(f, "Physical cores: %u\n", cpuInfo->physicalCores);
	fprintf(f, "Logical cores: %u\n", cpuInfo->logicalCores);
	fprintf(f, "L1 cache: %uKB\n", cpuInfo->L1Cache);
	fprintf(f, "L2 cache: %uKB\n", cpuInfo->L2Cache);
	fprintf(f, "L3 cache: %uKB\n", cpuInfo->L3Cache);
}

bool xtCPUHasHyperThreading(const struct xtCPUInfo *cpuInfo)
{
	return cpuInfo->logicalCores > cpuInfo->physicalCores;
}

int xtConsolePutString(const char *str, unsigned textWidth, bool hardSplit, enum xtConsoleAlignment alignment)
{
	int retval;
	unsigned cols;
	size_t size, rowsize, align;
	unsigned char *ptr, *next;

	retval = xtConsoleGetSize(&cols, NULL);

	if (retval)
		return retval;

	size = strlen(str);

	/* Check if specified alignment is correct */
	switch (alignment) {
	case XT_CONSOLE_ALIGN_LEFT:
	case XT_CONSOLE_ALIGN_CENTER:
	case XT_CONSOLE_ALIGN_RIGHT:
		break;
	default:
		return XT_EINVAL;
	}

	if (!textWidth)
		textWidth = cols;

	/* Check if the string is going to fit */
	if (size > textWidth)
		for (rowsize = 0, ptr = (unsigned char*)str; *ptr; ++ptr) {
			if (isspace(*ptr) || *ptr == '\n')
				rowsize = 0;
			else if (++rowsize >= textWidth && !hardSplit)
				return XT_EMSGSIZE;
		}

	for (ptr = (unsigned char*)str, next = ptr; *ptr; next = ptr) {
		for (rowsize = 0; *next; ++rowsize, ++next) {
			if (*next == '\n')
				break;

			if (rowsize >= textWidth) {
				if (!hardSplit)
					while (!isspace(*next))
						--next;
				break;
			}
		}

		switch (alignment) {
		case XT_CONSOLE_ALIGN_LEFT:
			align = 0;
			break;
		case XT_CONSOLE_ALIGN_CENTER:
			align = (cols - rowsize) / 2;
			break;
		case XT_CONSOLE_ALIGN_RIGHT:
			align = cols - rowsize;
			break;
		}

		while (align --> 0)
			putchar(' ');

		/* Print row */
		while (ptr < next)
			putchar(*ptr++);

		putchar('\n');

		while (isspace(*ptr))
			++ptr;
	}

	return 0;
}

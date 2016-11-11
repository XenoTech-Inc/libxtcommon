// XT headers
#include <xt/os.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <errno.h>
#include <fcntl.h> // for open() and O_RDONLY
#include <sys/ioctl.h> // for ioctl() and TIOCGWINSZ
#include <sys/stat.h>
#include <sys/sysinfo.h> // for the sysinfo function and struct
#include <termios.h>
#include <unistd.h> // for a ton of functions...

// STD headers
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

bool xtBatteryIsCharging(void)
{
	const char *chargingStr = "Charging";
	size_t chargingStrLen = strlen(chargingStr);
	char sbuf[64];
	for (int i = 0; i < 10; ++i) { // Check for max 10 batteries if they exist
		snprintf(sbuf, 64, "%s%d%s", "/sys/class/power_supply/BAT", i, "/status");
		int fd = open(sbuf, O_RDONLY);
		if (fd == -1)
			continue;
		int retval = read(fd, sbuf, 64);
		close(fd);
		return retval > 0 ? memcmp(chargingStr, sbuf, chargingStrLen) == 0 : false;
	}
	return false;
}

bool xtBatteryIsPresent(void)
{
	return xtBatteryGetPowerLevel() != -1;
}

int xtBatteryGetPowerLevel(void)
{
	char sbuf[64];
	for (int i = 0; i < 10; ++i) { // Check for max 10 batteries if they exist
		snprintf(sbuf, 64, "%s%d%s", "/sys/class/power_supply/BAT", i, "/capacity");
		int fd = open(sbuf, O_RDONLY);
		if (fd == -1)
			continue;
		int retval = read(fd, sbuf, 64);
		close(fd);
		return retval > 0 ? strtol(sbuf, NULL, 10) : -1;
	}
	return -1;
}

void xtCPUDump(const xtCPUInfo *cpuInfo, FILE *f)
{
	fprintf(f, "CPU name: %s\n", cpuInfo->name);
	char cpuArch[16];
	switch (cpuInfo->architecture) {
	case XT_CPU_ARCH_X64: strncpy(cpuArch, "x64", 16); break;
	case XT_CPU_ARCH_X86: strncpy(cpuArch, "x86", 16); break;
	case XT_CPU_ARCH_ARM: strncpy(cpuArch, "ARM", 16); break;
	case XT_CPU_ARCH_IA64: strncpy(cpuArch, "IA64", 16); break;
	default: strncpy(cpuArch, "Unknown", 16); break;
	}
	fprintf(f, "CPU architecture: %s\n", cpuArch);
	fprintf(f, "Physical cores: %u\n", cpuInfo->physicalCores);
	fprintf(f, "Logical cores: %u\n", cpuInfo->logicalCores);
	fprintf(f, "L1 cache: %uKB\n", cpuInfo->L1Cache);
	fprintf(f, "L2 cache: %uKB\n", cpuInfo->L2Cache);
	fprintf(f, "L3 cache: %uKB\n", cpuInfo->L3Cache);
}

bool xtCPUGetInfo(xtCPUInfo *cpuInfo)
{
	// If larger than zero, errors have occurred
	int errorCount = 0;
	// Initialize all values first to be safe
	strncpy(cpuInfo->name, "Unknown", sizeof(cpuInfo->name));
	cpuInfo->architecture = XT_CPU_ARCH_UNKNOWN;
	cpuInfo->physicalCores = 0;
	cpuInfo->logicalCores = 0;
	cpuInfo->L1Cache = 0;
	cpuInfo->L2Cache = 0;
	cpuInfo->L3Cache = 0;
	
	// Alternative code
#if 0
	char sbuf[1024]; // For the long flags line
	FILE *f = fopen("/proc/cpuinfo", "rb");
	unsigned cores = 1, siblings = 1, cpu_family = 0;
	if (f) {
		// Will contain the string for scanf
		char modelFormatStr[64];
		snprintf(modelFormatStr, sizeof(modelFormatStr), "model name : %%%luc", sizeof(cpuInfo->name));
		puts(modelFormatStr);
		while (fgets(sbuf, sizeof(sbuf), f)) {
			sbuf[strlen(sbuf) - 1] = '\0';
			if (sscanf(sbuf, "cpu family : %u", &cpu_family) == 1 && cpu_family >= 3 && cpu_family <= 6) // It's then always part of iX86
				cpuInfo->architecture = XT_CPU_ARCH_X86;
			if (strstr(sbuf, "flags")) {
				if (strstr(sbuf, "ia64")) // If the flags contains ia64, it's always itanium
					cpuInfo->architecture = XT_CPU_ARCH_IA64;
				else if (strstr(sbuf, " lm ")) // The processor is capable of long mode (64 bits)
					cpuInfo->architecture = XT_CPU_ARCH_X64;
			}
			if (sscanf(sbuf, modelFormatStr, cpuInfo->name) == 1) {
				// Sometimes the CPU name contains weird tabs or spaces. Remove them!
				xtStringTrimWords(cpuInfo->name);
			}
			sscanf(sbuf, "siblings : %u", &siblings);
			sscanf(sbuf, "cpu cores : %u", &cores);
		}
		cpuInfo->physicalCores = cores;
		cpuInfo->logicalCores = siblings;
		fclose(f);
	}
	for (unsigned i = 0; i < cpuInfo->logicalCores; ++i) {
		const char *base = "/sys/devices/system/cpu/cpu";
		unsigned cacheSizes[3] = { 0, 0, 0 }; // L1 cache is at index 0!
		for (unsigned j = 0; j < 4; ++j) {
			unsigned currLevel;
			snprintf(sbuf, sizeof(sbuf), "%s%u%s%u%s", base, i, "/cache/index", j, "/level");
			f = fopen(sbuf, "rb");
			if (f) {
				fgets(sbuf, sizeof(sbuf), f);
				currLevel = atoi(sbuf);
				fclose(f);
				snprintf(sbuf, sizeof(sbuf), "%s%u%s%u%s", base, i, "/cache/index", j, "/size");
				f = fopen(sbuf, "rb");
				if (!f)
					continue;
				fgets(sbuf, sizeof(sbuf), f);
				cacheSizes[currLevel - 1] += atoi(sbuf);
				fclose(f);
			}
		}
		cpuInfo->L1Cache = cacheSizes[0];
		cpuInfo->L2Cache = cacheSizes[1];
		cpuInfo->L3Cache = cacheSizes[2];
	}
#else
	char sbuf[128], sbuf2[128];
	char *tokens[4];
	unsigned numTokens;
	FILE *fp = popen("lscpu", "r");
	if (fp) {
		// Retrieve a lot of values!!
		while (xtStringReadLine(sbuf, sizeof(sbuf), NULL, fp)) {
			strncpy(sbuf2, sbuf, 128);
			numTokens = sizeof(tokens) / sizeof(tokens[0]);
			xtStringSplit(sbuf, " \t", tokens, &numTokens);
			if (xtStringStartsWith(sbuf, "Architecture")) {
				if (strcmp(tokens[1], "x86_64") == 0)
					cpuInfo->architecture = XT_CPU_ARCH_X64;
				else if (xtStringStartsWith(tokens[1], "arm"))
					cpuInfo->architecture = XT_CPU_ARCH_ARM;
				else if (xtStringEndsWith(tokens[1], "86"))
					cpuInfo->architecture = XT_CPU_ARCH_X86;
			} else if (xtStringStartsWith(sbuf, "CPU(s)"))
				cpuInfo->logicalCores = strtoul(tokens[1], NULL, 10);
			else if (xtStringStartsWith(sbuf, "Core(s)"))
				cpuInfo->physicalCores = strtoul(tokens[3], NULL, 10);
			else if (xtStringStartsWith(sbuf, "L1d")) {
				cpuInfo->L1Cache = strtoul(tokens[2], NULL, 10);
			} else if (xtStringStartsWith(sbuf, "L2")) {
				cpuInfo->L2Cache = strtoul(tokens[2], NULL, 10);
			} else if (xtStringStartsWith(sbuf, "L3")) {
				cpuInfo->L3Cache = strtoul(tokens[2], NULL, 10);
			}
		}
		pclose(fp);
		// We may have more than 1 core, so fix this value!
		// Times two, because there is L1d and L1i cache
		cpuInfo->L1Cache *= 2 * cpuInfo->physicalCores;
		cpuInfo->L2Cache *= cpuInfo->physicalCores;
	} else
		++errorCount;
		// Insurance. Fix the core count if this failed
	if (cpuInfo->physicalCores == 0 || cpuInfo->logicalCores == 0) {
		cpuInfo->physicalCores = 1;
		cpuInfo->logicalCores = 1;
	}
	fp = fopen("/proc/cpuinfo", "rb");
	if (fp) {
		while (xtStringReadLine(sbuf, sizeof(sbuf), NULL, fp)) {
			if (xtStringStartsWith(sbuf, "model name")) {
				strncpy(cpuInfo->name, strchr(sbuf, ':') + 2, sizeof(cpuInfo->name));
				break;
			}
		}
		fclose(fp);
		// Sometimes the CPU name contains weird tabs or spaces. Remove them!
		xtStringTrimWords(cpuInfo->name);
	} else
		++errorCount;
#endif
	return errorCount == 0;
}

bool xtCPUHasHyperThreading(const xtCPUInfo *cpuInfo)
{
	return cpuInfo->logicalCores > cpuInfo->physicalCores;
}

void xtConsoleClear(void)
{
	//system("clear && printf '"); // This is slow!
	// Alias for <ESC>c, the VT100 code to reset the terminal :D
	printf("%s", "\033c"); // Really ultra fast
}

int xtConsoleGetSize(unsigned *cols, unsigned *rows)
{
	if (!xtConsoleIsAvailable())
		return XT_EINVAL;
	struct winsize ws;
	// Initialize these values to be safe
	ws.ws_row = 0;
	ws.ws_col = 0;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
		return _xtTranslateSysError(errno);
	if (ws.ws_row == 0 && ws.ws_col == 0)
		return _xtTranslateSysError(errno);
	if (cols)
		*cols = ws.ws_col;
	if (rows)
		*rows = ws.ws_row;
	return 0;
}

bool xtConsoleIsAvailable(void)
{
	return getenv("TERM"); // Should be NULL when there is no console
}

bool xtConsoleSetTitle(const char *title)
{
	char buf[512];
	snprintf(buf, 512, "%s%s%s", "printf \"\033]0;", title, "\007\"");
	return system(buf) != -1;
}

unsigned long long xtRAMGetAmountFree(void)
{
	/* Doesn't show correct amount of RAM on 32 bit systems with 4GB+ RAM
	struct sysinfo info;
	sysinfo(&info); // Cannot fail
	return info.freeram;
	*/
	FILE *f = fopen("/proc/meminfo", "rb");
	if (!f)
		return 0;
	char sbuf[128];
	unsigned long long amount;
	while (fgets(sbuf, sizeof(sbuf), f)) {
		if (sscanf(sbuf, "MemFree: %llu kB", &amount) == 1) {
			fclose(f);
			return amount * 1024;
		}
	}
	fclose(f);
	return 0;
}

unsigned long long xtRAMGetAmountTotal(void)
{
	/* Doesn't show correct amount of RAM on 32 bit systems with 4GB+ RAM
	struct sysinfo info;
	sysinfo(&info); // Cannot fail
	return info.totalram;
	*/
	FILE *f = fopen("/proc/meminfo", "rb");
	if (!f)
		return 0;
	char sbuf[128];
	unsigned long long amount;
	while (fgets(sbuf, sizeof(sbuf), f)) {
		if (sscanf(sbuf, "MemTotal: %llu kB", &amount) == 1) {
			fclose(f);
			return amount * 1024;
		}
	}
	fclose(f);
	return 0;
}

unsigned xtGetCurrentPID(void)
{
	return getpid();
}

char *xtGetHostname(char *buf, size_t buflen)
{
	// For in the future maybe, HOST_NAME_MAX is the maximum length of the hostname in limits.h
	if (gethostname(buf, buflen) != 0)
		return NULL;
	return buf;
}

char *xtGetOSName(char *buf, size_t  buflen)
{
	FILE *fp = popen("/usr/bin/lsb_release -d | /bin/sed -e 's/.*:\\s//'", "r");
	if (!fp || !xtStringReadLine(buf, buflen, NULL, fp)) {
		snprintf(buf, buflen, "%s", "Linux");
		return buf;
	}
	fclose(fp);
	return buf;
}

unsigned xtGetProcessCount(void)
{
	char line[31];
	FILE *fp = popen("ps -fe | wc -l", "r");
	if (!fp || !fgets(line, 31, fp))
		return 0;
	pclose(fp);
	return atoi(line);
}

char *xtGetUsername(char *buf, size_t buflen)
{
	// For in the future maybe, LOGIN_NAME_MAX is the maximum length of the name. in limits.h
	if (getlogin_r(buf, buflen) == 0)
		return buf;
	return NULL;
}

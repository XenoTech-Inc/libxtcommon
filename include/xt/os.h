/**
 * @brief Used to retrieve deep system information.
 *
 * All functions in here are used to retrieve information about the
 * system's hardware and OS information.
 * @file os.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_OS_H
#define _XT_OS_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/**
 * Returns if the battery is charging now. False is returned on error.
 * @remarks If the battery is fully charged, the system will usually
 * report that the battery is NOT charging.
 */
bool xtBatteryIsCharging(void);
/**
 * Returns if there is a battery present in the system.
 */
bool xtBatteryIsPresent(void);
/**
 * Returns the percentage of the battery's power level.
 * @returns -1 If no battery is present or if there was an error reading
 * the power level. Otherwise the power level is returned in the usual
 * range from 0% to 100%.
 */
int xtBatteryGetPowerLevel(void);
/**
 * The available CPU architectures that can be detected.
 */
enum xtCPUArch {
	/* Unknown CPU architecture */
	XT_CPU_ARCH_UNKNOWN,
	/** x64 from AMD or Intel */
	XT_CPU_ARCH_X64,
	/** x86, which includes i386, i586 and i686 */
	XT_CPU_ARCH_X86,
	/** Any of the ARM architecture versions */
	XT_CPU_ARCH_ARM,
	/** IA-64, the Intel Itanium architecture */
	XT_CPU_ARCH_IA64
};
/**
 * Contains a lot of information about the CPU.
 */
struct xtCPUInfo {
	/**
	 * The name of the processor. This is likely to also contain the
	 * manufacterer name and clock frequency.
	 */
	char name[64];
	/**
	 * The architecture of the processor.
	 */
	enum xtCPUArch architecture;
	/**
	 * The amount of cores that your CPU possesses.
	 */
	unsigned physicalCores, logicalCores;
	/**
	 * Processor cache sizes in bytes.
	 */
	unsigned L1Cache, L2Cache, L3Cache;
};
/**
 * Dumps the processor information to the specified stream.
 */
void xtCPUDump(const struct xtCPUInfo *restrict cpuInfo, FILE *restrict f);
/**
 * Retrieves information about the processor. All fields will ALWAYS be filled
 * with default values, even although an error may occur while retrieving some
 * info. This means that it is possible that not all information will be
 * retrieved.
 * @return True if all information has successfully been retrieved. False is
 * returned if the information has been retrieved only partially.
 * @remarks Not reliable when having multiple sockets installed with running
 * processors or when ran in a VM. The results will probably be inaccurate.\n
 * Problems when compiling for Windows 32 bit: The info will always be retrieved
 * partially. The amount of physical cores will always be same as the amount
 * of logical cores.
 */
bool xtCPUGetInfo(struct xtCPUInfo *cpuInfo);
/**
 * Returns if this processor has some sort of HyperThreading enabled.
 * (Multiple threads per core)
 */
bool xtCPUHasHyperThreading(const struct xtCPUInfo *cpuInfo);
/**
 * Clears the console screen.
 */
void xtConsoleClear(void);
/**
 * Fills row with pattern by repeating it as many times as possible.
 * This function takes care of the new line problem of each platform's console.
 */
int xtConsoleFillLine(const char *pattern);
/**
 * Tells you the available amount of columns and rows in the console.
 * Both pointers are optional. They remain untouched on error.
 * @return Zero if the property has been fetched, otherwise an error code.
 */
int xtConsoleGetSize(unsigned *restrict cols, unsigned *restrict rows);
/**
 * Returns if the program was launched from a console and thus if one is
 * available. If the program was launched by double-clicking, false should
 * be returned.
 */
bool xtConsoleIsAvailable(void);
/**
 * Changes the console title. This will only have effect if stdout is
 * associated with a terminal, otherwise it's a NOP.
 */
void xtConsoleSetTitle(const char *title);
/**
 * Returns the hostname of this computer. On error a null pointer is returned.
 */
char *xtGetHostname(char *buf, size_t buflen);
/**
 * Returns the name of the OS that this computer is running. On failure, a less
 * accurate representation of the OS name is returned.
 * (e.g "Linux" instead of "Ubuntu 14.04.5 LTS")
 * @remarks For Linux, this only works on LSB compliant distros.
 */
char *xtGetOSName(char *buf, size_t buflen);
/**
 * Contains information about the RAM usage of the system.
 */
struct xtRAMInfo {
	/**
	 * The total amount of physical RAM available for use in bytes.
	 */
	unsigned long long free;
	/**
	 * The total amount of physical RAM available to the system in bytes.
	 * @remarks This value will differ across operating systems. This function
	 * will never display the "true" installed RAM value.
	 */
	unsigned long long total;
};
/**
 * Fetches all the RAM information from the system.
 * @return Zero if the information has been fetched, otherwise an error code.
 */
int xtRAMGetInfo(struct xtRAMInfo *ramInfo);
/**
 * Returns the name of the user who is logged in on this session. On error a
 * null pointer is returned.
 */
char *xtGetUsername(char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif

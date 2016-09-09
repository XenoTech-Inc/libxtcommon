/**
 * @brief Used to retrieve deep system information.
 * 
 * All functions in here are used to retrieve information about the system's hardware.
 * @file os.h
 * @author Tom Everaarts
 * @date 2016
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
 * @remarks If the battery is fully charged, the system will usually report that the battery is NOT charging.
 */
bool xtBatteryIsCharging(void);
/**
 * Returns if there is a battery present in the system.
 */
bool xtBatteryIsPresent(void);
/**
 * Returns the percentage of the battery's power level.
 * @returns -1 If no battery is present or there was an error reading the power level.\n
 * Otherwise the power level is returned in the usual range from 0% to 100%.
 */
int xtBatteryGetPowerLevel(void);
/**
 * The available CPU architectures that can be detected.
 */
typedef enum xtCPUArch {
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
} xtCPUArch;
/**
 * Contains a lot of information about the CPU.
 */
typedef struct xtCPU {
	/**
	 * The name of the processor. This is likely to also contain the manufacterer name 
	 * and clock frequency.
	 */
	char name[64];
	/**
	 * The architecture of the processor.
	 */
	xtCPUArch architecture;
	/**
	 * The amount of cores that your CPU possesses.
	 */
	unsigned physicalCores, logicalCores;
	/**
	 * Processor cache sizes in bytes.
	 */
	unsigned L1Cache, L2Cache, L3Cache;
} xtCPU;
/**
 * Dumps the processor information to the specified stream.
 */
void xtCPUDump(const xtCPU *cpuInfo, FILE *f);
/**
 * Retrieves information about the processor. All fields will ALWAYS be filled with default values, 
 * even although an error may occur while retrieving some info. This means that it is possible that not all 
 * information will be retrieved.
 * @return True if all information has successfully been retrieved. False is returned 
 * if the information has been retrieved only partially.
 * @remarks Not reliable when having multiple sockets installed with running processors or when ran in a VM. 
 * The results will probably be inaccurate.\n
 * Problems when compiling for Windows 32 bit: The info will always be retrieved partially. The amount of 
 * physical cores will always be same as the amount of logical cores.
 */
bool xtCPUGetInfo(xtCPU *cpuInfo);
/**
 * Returns if this processor has some sort of HyperThreading enabled. (Multiple threads per core)
 */
bool xtCPUHasHyperThreading(const xtCPU *cpuInfo);
/**
 * Clears the console screen.
 */
void xtConsoleClear(void);
/**
 * Returns if the program was launched from a console and thus if one is available.
 */
bool xtConsoleIsAvailable(void);
/**
 * Changes the console title.
 */
bool xtConsoleSetTitle(const char *title);
/**
 * Returns the total amount of physical RAM available for use in bytes. 
 * Zero is returned on failure.
 */
unsigned long long xtRAMGetAmountFree(void);
/**
 * Returns the total amount of physical RAM available to the system in bytes. 
 * Zero is returned on failure.
 * @remarks This value will differ across operating systems. This function will never display the "true" installed RAM value.
 */
unsigned long long xtRAMGetAmountTotal(void);
/**
 * Returns the hostname of this computer. On error a null pointer is returned.
 */
char *xtGetHostname(char *buf, size_t buflen);
/**
 * Returns the name of the OS that this computer is running. On error a null pointer is returned.
 * @remarks For Linux, this only works on LSB compliant distros.
 */
char *xtGetOSName(char *buf, size_t buflen);
/**
 * Returns the name of the user who is logged in on this session. On error a null pointer is returned.
 */
char *xtGetUsername(char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif

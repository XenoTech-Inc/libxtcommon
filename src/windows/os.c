// XT headers
#include <xt/os.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <Windows.h> // Include windows.h before TlHelp32.h, otherwise we get problems
#include <TlHelp32.h> // For CreateToolhelp32Snapshot
#ifdef _WIN64
	#include <intrin.h> // For the cpuid function, only available on 64 bit...
#else
	#include <x86intrin.h>
#endif

// STD headers
#include <stdlib.h>
#include <string.h>

bool xtBatteryIsCharging(void)
{
	SYSTEM_POWER_STATUS status;
	// Initialize these values so that we're safe later on
	status.BatteryLifePercent = 0;
	status.BatteryFlag = 0;
	if (GetSystemPowerStatus(&status) == 0)
		return false;
	if (status.BatteryLifePercent == 255 || status.BatteryFlag == 128 || status.BatteryFlag == 255)
		return false;
	return status.BatteryFlag & 8;
}

bool xtBatteryIsPresent(void)
{
	return xtBatteryGetPowerLevel() != -1;
}

int xtBatteryGetPowerLevel(void)
{
	SYSTEM_POWER_STATUS status;
	// Initialize these values so we're safe later on
	status.BatteryLifePercent = 0;
	status.BatteryFlag = 0;
	if (GetSystemPowerStatus(&status) == 0)
		return -1;
	if (status.BatteryLifePercent == 255 || status.BatteryFlag == 128 || status.BatteryFlag == 255)
		return -1;
	return status.BatteryLifePercent;
}

void xtCPUDump(const struct xtCPUInfo *cpuInfo, FILE *f)
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

bool xtCPUGetInfo(struct xtCPUInfo *cpuInfo)
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
	
	// Fetch the CPU architecture
	SYSTEM_INFO sysInfo;
	GetNativeSystemInfo(&sysInfo); // Cannot fail
	switch (sysInfo.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_AMD64: cpuInfo->architecture = XT_CPU_ARCH_X64; break;
	case PROCESSOR_ARCHITECTURE_INTEL: cpuInfo->architecture = XT_CPU_ARCH_X86; break;
	case PROCESSOR_ARCHITECTURE_ARM: cpuInfo->architecture = XT_CPU_ARCH_ARM; break;
	case PROCESSOR_ARCHITECTURE_IA64: cpuInfo->architecture = XT_CPU_ARCH_IA64; break;
	default: cpuInfo->architecture = XT_CPU_ARCH_UNKNOWN; break;
	}
	cpuInfo->logicalCores = sysInfo.dwNumberOfProcessors;
	// Fetch the CPU name
	// Only works on 64 bit windows... __cpuid is not available on 32 bit Windows
	char sbuf[128];
	int CPUInfo[4] = {-1};
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];
	// Get the information associated with each extended ID.
	for (unsigned i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(CPUInfo, i);
	// Interpret CPU brand string
	if (i >= 0x80000002 && i <= 0x80000004)
		memcpy(sbuf + 16 * (i - 0x80000002), CPUInfo, sizeof(CPUInfo));
	}
	// For some reason there are a lot of spaces infront; filter them out.
	char *spaces = sbuf;
	while (*spaces == ' ') ++spaces;
	// Sometimes the CPU name contains weird tabs or spaces. Remove them!
	xtStringTrimWords(sbuf);
	snprintf(cpuInfo->name, sizeof(cpuInfo->name), "%s", sbuf);
	// Fetch the amount of logical cores
	typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);
	LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetLogicalProcessorInformation");
	if (glpi) {
		// Fetch the amount of cores per socket
		DWORD buffer_size = 0;
		DWORD i = 0;
		SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;
		GetLogicalProcessorInformation(0, &buffer_size);
		buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) malloc(buffer_size);
		
		int processorL1CacheCount = 0, processorL2CacheCount = 0, processorL3CacheCount = 0;
		int processorL1CacheSize = 0, processorL2CacheSize = 0, processorL3CacheSize = 0;
		if (buffer) {
			GetLogicalProcessorInformation(&buffer[0], &buffer_size);
			for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
				if (buffer[i].Relationship == RelationProcessorCore)
					cpuInfo->physicalCores++;
			}
			// Count one extra. Otherwise the number will be off
			++cpuInfo->physicalCores;
			// Fetch the cache info
			CACHE_DESCRIPTOR *cache;
			for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
				switch (buffer[i].Relationship) {
				case RelationCache:
					cache = &buffer[i].Cache;
					if (cache->Level == 1) {
						processorL1CacheCount++;
						processorL1CacheSize = cache->Size;
					} else if (cache->Level == 2) {
						processorL2CacheCount++;
						processorL2CacheSize = cache->Size;
					} else if (cache->Level == 3) {
						processorL3CacheCount++;
						processorL3CacheSize = cache->Size;
					}
					break;
				default:
					// Otherwise the compiler will warn that we are not inspecting all fields
					break;
				}
			}
			free(buffer);
		} else
			++errorCount;
		// This is 1 too much all the time for some reason, decrease it!
		--cpuInfo->physicalCores;
		// Divide it by 1024 to go from bytes to KB
		cpuInfo->L1Cache = processorL1CacheSize * processorL1CacheCount / 1024;
		cpuInfo->L2Cache = processorL2CacheSize * processorL2CacheCount / 1024;
		cpuInfo->L3Cache = processorL3CacheSize * processorL3CacheCount / 1024;
	} else
		++errorCount;
	// Insurance. Fix the core count if this failed
	if (cpuInfo->physicalCores == 0 || cpuInfo->logicalCores == 0) {
		cpuInfo->physicalCores = 1;
		cpuInfo->logicalCores = 1;
	}
	return errorCount == 0;
}

bool xtCPUHasHyperThreading(const struct xtCPUInfo *cpuInfo)
{
	return cpuInfo->logicalCores > cpuInfo->physicalCores;
}

void xtConsoleClear(void)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (handle == INVALID_HANDLE_VALUE)
		return;
	COORD coordScreen = { 0, 0 }; // This will be the cursor position after clearing the console
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; // Will receive the buffer info
	DWORD dwConSize; // Number of character cells in the current buffer
	GetConsoleScreenBufferInfo(handle, &csbi); // Get the number of character cells in the current buffer
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(handle, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten); // Fill the screen with spaces
	GetConsoleScreenBufferInfo(handle, &csbi); // Get current text attribute
	FillConsoleOutputAttribute(handle, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten); // Set the buffer attributes
	SetConsoleCursorPosition(handle, coordScreen); // Reset the cursor position
}

int xtConsoleGetSize(unsigned *cols, unsigned *rows)
{
	if (!xtConsoleIsAvailable())
		return XT_EINVAL;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi) == FALSE)
		return XT_EINVAL;
	if (cols)
		*cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	if (rows)
		*rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return 0;
}

bool xtConsoleIsAvailable(void)
{
	HWND consoleWnd = GetConsoleWindow();
	DWORD dwProcessId;
	GetWindowThreadProcessId(consoleWnd, &dwProcessId);
	return GetCurrentProcessId() != dwProcessId;
}

bool xtConsoleSetTitle(const char *title)
{
	return SetConsoleTitle(title);
}

unsigned long long xtRAMGetAmountFree(void)
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	// Cannot fail, as we are setting this all up correctly
	GlobalMemoryStatusEx(&statex);
	return statex.ullAvailPhys;
}

unsigned long long xtRAMGetAmountTotal(void)
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	// Cannot fail, as we are setting this all up correctly
	GlobalMemoryStatusEx(&statex);
	return statex.ullTotalPhys;
}

unsigned xtGetCurrentPID(void)
{
	return GetCurrentProcessId();
}

char *xtGetHostname(char *buf, size_t buflen)
{
	char *val = getenv("COMPUTERNAME");
	if (!val)
		return NULL;
	snprintf(buf, buflen, "%s", val);
	return buf;
}
/**
 * Special function. Since Windows 8.1 GetVersion is kind of broken and won't work correctly.
 * To avoid that, Microsoft recommends changing stuff in the resource file or including some header which nobody understands how to even do.
 * $hit$oft just breaks their API with a ridiculously bad new one, but this special function avoids doing that stupid crap. It retrieves the correct version.
 * It should also work on Windows 2000.
 */
static bool _xtGetWindowsVersion(DWORD *major, DWORD *minor)
{
#if !defined(RTL_OSVERSIONINFOEXW)
	typedef struct RTL_OSVERSIONINFOEXW {
		DWORD dwOSVersionInfoSize, dwMajorVersion, dwMinorVersion,
		dwBuildNumber, dwPlatformId;
		char szCSDVersion[128];
		WORD wServicePackMajor,wServicePackMinor, wSuiteMask, wProductType, wReserved;
	} RTL_OSVERSIONINFOEXW;
#endif
	typedef LONG (WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
	RTL_OSVERSIONINFOEXW pk_OsVer;
	memset(&pk_OsVer, 0, sizeof(RTL_OSVERSIONINFOEXW));
	pk_OsVer.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	HMODULE h_NtDll = GetModuleHandle(TEXT("ntdll.dll"));
	tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion) GetProcAddress(h_NtDll, "RtlGetVersion");
	if (!f_RtlGetVersion)
		return false; // This will never happen (all processes load ntdll.dll)
	if (f_RtlGetVersion(&pk_OsVer) == 0) {
		*major = pk_OsVer.dwMajorVersion;
		*minor = pk_OsVer.dwMinorVersion;
		return true;
	} else
		return false;
}

char *xtGetOSName(char *buf, size_t buflen)
{
	char *os = buf;
	#define APPEND(str) os += snprintf(os, buflen - (os - buf), str)
	APPEND("Windows");
	DWORD major, minor;
	if (!_xtGetWindowsVersion(&major, &minor))
		return buf;
	APPEND(" ");
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO) &osvi);
	bool isServer = osvi.wProductType != VER_NT_WORKSTATION;
	switch (major) {
		case 10:
			if (isServer) APPEND("Server 2016");
			else APPEND("10");
			break;
		case 6:
			if (minor == 3 && isServer) APPEND("Server 2012 R2");
			else if (minor == 3) APPEND("8.1");
			else if (minor == 2 && isServer) APPEND("Server 2012");
			else if (minor == 2) APPEND("8");
			else if (minor == 1 && isServer) APPEND("Server 2008 R2");
			else if (minor == 1) APPEND("7");
			else if (minor == 0 && isServer) APPEND("Server 2008");
			else if (minor == 0) APPEND("Vista");
			break;
		case 5:
			if (minor == 2 && isServer) APPEND("Server 2003");
			else if (minor == 2) APPEND("XP 64-Bit Edition");
			else if (minor == 1) APPEND("XP");
			else if (minor == 0) APPEND("2000");
			break;
	}
	#undef APPEND
	*os = '\0';
	return buf;
}

unsigned xtGetProcessCount(void) {
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return 0;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return 0;
	}
	unsigned processCount = 0;
	while (Process32Next(hProcessSnap, &pe32)) ++processCount;
	CloseHandle(hProcessSnap);
	return processCount;
}

char *xtGetUsername(char *buf, size_t buflen)
{
	char *val = getenv("USERNAME");
	if (!val)
		return NULL;
	snprintf(buf, buflen, "%s", val);
	return buf;
}

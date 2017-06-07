// XT headers
#include <xt/proc.h>
#include <xt/error.h>
#include <xt/string.h>

// System headers
#include <windows.h> // Include windows.h before psapi.h and tlhelp32.h
#include <psapi.h> // For GetProcessMemoryInfo()
#include <tlhelp32.h> // For CreateToolhelp32Snapshot()

// STD headers
#include <stdbool.h>
#include <stdio.h>

#define FileTimeToLLU(x) ((((ULONGLONG) (x).dwHighDateTime) << 32) + (x).dwLowDateTime)

unsigned xtProcGetCurrentPID(void)
{
	return GetCurrentProcessId();
}

float xtProcCPUTimeCalculate(const struct xtProcCPUTime *start,
	const struct xtProcCPUTime *end)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo); // Cannot fail
	float percent = (end->stime - start->stime) + (end->utime - start->utime);
	percent /= (end->timestamp - start->timestamp);
	percent /= (float) sysInfo.dwNumberOfProcessors;
	return percent * 100;
}

int xtProcGetCPUTime(unsigned pid, struct xtProcCPUTime *cpuTime)
{
	int ret = 1;
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!handle)
		goto error;
	FILETIME ftTime, ftSys, ftUser;
	GetSystemTimeAsFileTime(&ftTime);
	cpuTime->timestamp = FileTimeToLLU(ftTime);
	GetProcessTimes(handle, &ftTime, &ftTime, &ftSys, &ftUser);
	cpuTime->stime = FileTimeToLLU(ftSys);
	cpuTime->utime = FileTimeToLLU(ftUser);
	CloseHandle(handle);
	ret = 0;
error:
	return ret == 0 ? 0 : _xtTranslateSysError(GetLastError());
}

int xtProcGetMemoryInfo(unsigned pid, struct xtProcMemoryInfo *info)
{
	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!handle)
		goto error;
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(handle, &pmc, sizeof pmc) == FALSE) {
		CloseHandle(handle);
		goto error;
	}
	info->hwm = pmc.PeakWorkingSetSize;
	info->rss = pmc.WorkingSetSize;
	// Fucking can't be done reliably!!!. The API that $hit$oft offers just
	// doesn't work. The returned numbers are bogus so they are no use.
	info->swap = 0;
	info->vmPeak = 0;
	CloseHandle(handle);
	return 0;
error:
	return _xtTranslateSysError(GetLastError());
}

int xtProcGetName(unsigned pid, char *buf, size_t buflen)
{
	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!handle)
		goto error;
	DWORD len = buflen;
	if (QueryFullProcessImageName(handle, 0, buf, &len) == FALSE) {
		CloseHandle(handle);
		goto error;
	}
	xtStringReplaceAll(buf, '\\', '/');
	CloseHandle(handle);
	return 0;
error:
	return _xtTranslateSysError(GetLastError());
}

int xtProcGetPids(unsigned *restrict pids, unsigned *restrict pidCount)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return _xtTranslateSysError(GetLastError());
	pe32.dwSize = sizeof pe32;
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return _xtTranslateSysError(GetLastError());
	}
	unsigned currentPidCount = 0;
	unsigned maxPidCount = *pidCount;
	while (currentPidCount < maxPidCount && Process32Next(hProcessSnap, &pe32)) {
		pids[currentPidCount] = pe32.th32ProcessID;
		++currentPidCount;
	}
	CloseHandle(hProcessSnap);
	*pidCount = currentPidCount;
	return 0;
}

unsigned xtProcGetProcessCount(void)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return 0;
	pe32.dwSize = sizeof pe32;
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return 0;
	}
	unsigned processCount = 0;
	while (Process32Next(hProcessSnap, &pe32))
		++processCount;
	CloseHandle(hProcessSnap);
	return processCount;
}

bool xtProcIsAlive(unsigned pid)
{
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!handle) {
		// Process is alive but we don't have access to open a handle to it
		return GetLastError() == ERROR_ACCESS_DENIED ? true : false;
	}
	CloseHandle(handle);
	return true;
}

int xtProcKill(unsigned pid, enum xtProcSignal signal)
{
	char sbuf[128];
	HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (!handle)
		return _xtTranslateSysError(GetLastError());
	// Windows doesn't really support signals so just choose
	// the most appropriate signal to use
	int ret;
	if (signal == XT_SIGKILL)
		ret = TerminateProcess(handle, signal) == TRUE ? 0 : _xtTranslateSysError(GetLastError());
	else {
		snprintf(sbuf, sizeof sbuf, "C:\\windows\\system32\\taskkill.exe /t /pid %u > NUL", pid);
		ret = system(sbuf) == 0 ? 0 : _xtTranslateSysError(GetLastError());
	}
	CloseHandle(handle);
	return ret;
}

int xtProcWait(unsigned pid, unsigned timeout)
{
	HANDLE handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
	if (!handle)
		return _xtTranslateSysError(GetLastError());
	if (timeout == 0)
		timeout = INFINITE;
	int ret = WaitForSingleObject(handle, timeout);
	switch (ret) {
	case WAIT_OBJECT_0: return 0;
	case WAIT_TIMEOUT:  return XT_ETIMEDOUT;
	case WAIT_FAILED:   return _xtTranslateSysError(GetLastError());
	default:			return XT_EUNKNOWN;
	}
}

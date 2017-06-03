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

unsigned xtProcGetCurrentPID(void)
{
	return GetCurrentProcessId();
}

int xtProcGetMemoryInfo(struct xtProcMemoryInfo *info, unsigned pid)
{
	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (handle == NULL)
		goto error;
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(handle, &pmc, sizeof(pmc)) == FALSE) {
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

int xtProcGetName(char *buf, size_t buflen, unsigned pid)
{
	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (handle == NULL)
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
	pe32.dwSize = sizeof(PROCESSENTRY32);
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
	pe32.dwSize = sizeof(PROCESSENTRY32);
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
	if (handle == NULL) {
		// Process is alive but we don't have access to open a handle to it
		return GetLastError() == ERROR_ACCESS_DENIED ? true : false;
	}
	CloseHandle(handle);
	return true;
}

int xtProcKill(unsigned pid, enum xtProcSignal signal)
{
	char sbuf[128];
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (handle == NULL)
		return _xtTranslateSysError(GetLastError());
	int ret;
	if (signal == XT_SIGKILL)
		ret = TerminateProcess(handle, signal) == TRUE ? 0 : _xtTranslateSysError(GetLastError());
	else {
		snprintf(sbuf, sizeof(sbuf), "C:\\windows\\system32\\taskkill.exe /t /pid %u > NUL", pid);
		ret = system(sbuf) == 0 ? 0 : _xtTranslateSysError(GetLastError());
	}
	CloseHandle(handle);
	return ret;
}

int xtProcWait(unsigned pid, unsigned timeout)
{
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (handle == NULL)
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

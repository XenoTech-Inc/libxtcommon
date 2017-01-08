// XT headers
#include <xt/proc.h>
#include <xt/error.h>

// System headers
#include <Windows.h> // Include windows.h before TlHelp32.h, otherwise we get problems
#include <TlHelp32.h> // For CreateToolhelp32Snapshot

// STD headers
#include <stdbool.h>
#include <stdio.h>

unsigned xtProcGetCurrentPID(void)
{
	return GetCurrentProcessId();
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
	while (Process32Next(hProcessSnap, &pe32)) ++processCount;
	CloseHandle(hProcessSnap);
	return processCount;
}

bool xtProcIsAlive(unsigned pid)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return false;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return false;
	}
	bool isAlive = false;
	while (Process32Next(hProcessSnap, &pe32)) {
		if (pe32.th32ProcessID == pid) {
			isAlive = true;
			break;
		}
	}
	CloseHandle(hProcessSnap);
	return isAlive;
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
	int ret = WaitForSingleObject(handle, timeout);
	switch (ret) {
	case WAIT_OBJECT_0: return 0;
	case WAIT_TIMEOUT:  return XT_ETIMEDOUT;
	case WAIT_FAILED:   return _xtTranslateSysError(GetLastError());
	default:			return XT_EUNKNOWN;
	}
}

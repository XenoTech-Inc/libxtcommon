/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/dlload.h>

// System headers
#include <windows.h>

void xtDLClose(xtDLHandle handle)
{
	FreeLibrary((HMODULE)handle);
}

char *xtDLError(char *buf, size_t buflen)
{
	if (GetLastError() == ERROR_SUCCESS)
		return NULL;

	DWORD retval = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, buflen, NULL);

	SetLastError(ERROR_SUCCESS); // Clear any old error

	return retval > 0 ? buf : NULL;
}

void *xtDLGetProcAddress(xtDLHandle handle, const char *symbol)
{
	SetLastError(ERROR_SUCCESS); // Clear any old error
	FARPROC ptr = GetProcAddress((HMODULE)handle, symbol);
	return *(void**)&ptr;
}

xtDLHandle xtDLOpen(const char *path, enum xtDLFlag flag)
{
	(void)flag;
	return LoadLibrary(path);
}

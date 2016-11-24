// XT headers
#include <xt/dlload.h>

// System headers
#include <windows.h>

void *xtDLOpen(const char *filename, enum xtDLFlag flag)
{
	(void) flag;
	return (void*) LoadLibrary(filename);
}

void xtDLClose(void *handle)
{
	FreeLibrary((HMODULE) handle);
}

char *xtDLError(char *buf, size_t buflen)
{
	if (GetLastError() == ERROR_SUCCESS)
		return NULL;
	DWORD retval = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, buflen, NULL);
	return retval > 0 ? buf : NULL;
}

xtGenericFuncPtr xtDLSym(void *handle, const char *symbol)
{
	// Clear any old error
	SetLastError(ERROR_SUCCESS);
	return (xtGenericFuncPtr) GetProcAddress((HMODULE) handle, symbol);
}

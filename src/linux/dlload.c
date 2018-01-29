/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

// XT headers
#include <xt/dlload.h>

// System headers
#include <dlfcn.h>

// STD headers
#include <stdio.h> // snprintf

void xtDLClose(xtDLHandle handle)
{
	dlclose(handle);
}

char *xtDLError(char *buf, size_t buflen)
{
	char *error = dlerror(); // This clears any errors also
	if (!error)
		return NULL;

	snprintf(buf, buflen, "%s", error);
	return buf;
}

void *xtDLGetProcAddress(xtDLHandle handle, const char *symbol)
{
	dlerror(); // Clear any old error
	return dlsym(handle, symbol);
}

xtDLHandle xtDLOpen(const char *path, enum xtDLFlag flag)
{
	int _flag;
	if (flag == XT_DL_LAZY)
		_flag = RTLD_LAZY;
	else if (flag == XT_DL_NOW)
		_flag = RTLD_NOW;
	else
		_flag = RTLD_LAZY;

	return dlopen(path, _flag);
}

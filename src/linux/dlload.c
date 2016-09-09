// XT headers
#include <xt/dlload.h>

// System headers
#include <dlfcn.h>

// STD headers
#include <stdio.h> // snprintf

void *xtDLOpen(const char *filename, xtDLFlag flag)
{
	int _flag;
	if (flag == XT_DL_LAZY)
		_flag = RTLD_LAZY;
	else if (flag == XT_DL_NOW)
		_flag = RTLD_NOW;
	else
		_flag = RTLD_LAZY;
	return dlopen(filename, _flag);
}

void xtDLClose(void *handle)
{
	dlclose(handle);
}

char *xtDLError(char *buf, size_t buflen)
{
	char *error = dlerror();
	if (!error)
		return NULL;
	snprintf(buf, buflen, "%s", error);
	return buf;
}

xtGenericFuncPtr xtDLSym(void *handle, const char *symbol)
{
	// Clear any old error
	dlerror();
	/*union _utemp {
		void *ptr;
		xtGenericFuncPtr fptr;
	} utemp;
	utemp.ptr = dlsym(handle, symbol);
	return utemp.fptr;*/
	// We need to do this in 2 steps to make it safe
	void *(*fptr) (void*);
	*(void **) (&fptr) = dlsym(handle, symbol);
	return (xtGenericFuncPtr) fptr;
}

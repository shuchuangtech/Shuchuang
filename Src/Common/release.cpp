#include "release.h"
const char* getMKTIME()
{
#ifdef SC_MKTIME
	return SC_MKTIME;
#else
	return 0;
#endif
}

const char* getGITSHA1()
{
#ifdef SC_GIT_SHA1
	return SC_GIT_SHA1;
#else
	return 0;
#endif
}

const char* getGITDIRTY()
{
#ifdef SC_GIT_DIRTY
	return SC_GIT_DIRTY;
#else
	return 0;
#endif
}


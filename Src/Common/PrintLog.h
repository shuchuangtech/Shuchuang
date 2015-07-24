#ifndef __COMMON_PRINTLOG_H__
#define __COMMON_PRINTLOG_H__
#include "stdio.h"
enum
{
	LEVEL_FATAL = 1,
	LEVEL_ERROR = 2,
	LEVEL_WARN = 3,
	LEVEL_INFO = 4,
	LEVEL_TRACE = 5,
	LEVEL_DEBUG = 6
};
enum
{
	COLOR_FATAL = 35,
	COLOR_ERROR = 31,
	COLOR_WARN = 33,
	COLOR_INFO = 32,
	COLOR_TRACE = 37,
	COLOR_DEBUG = 36
};
#define tracepoint() debugf("tracepoint: %s, %d.\n", __FILE__, __LINE__)
bool setPrintLogLevel(int logLevel);
int debugf(const char* fmt, ... );
int infof(const char* fmt, ...);
int tracef(const char* fmt, ...);
int warnf(const char* fmt, ...);
int errorf(const char* fmt, ...);
int fatalf(const char* fmt, ...);
#endif


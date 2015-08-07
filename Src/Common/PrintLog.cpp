#include "Common/PrintLog.h"
#include <stdarg.h>
#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/FileChannel.h"
#include "Poco/AutoPtr.h"

static Poco::AutoPtr<Poco::FileChannel> pChannel;
static int s_printLogLevel = LEVEL_DEBUG;

inline void set_console_color(int c)
{
	fprintf(stdout, "\033[%d;40m", c);
}

inline void reset_console_color()
{
	fprintf(stdout, "\033[0m");
}

bool initPrintLogger()
{
	pChannel = new Poco::FileChannel;
	pChannel->setProperty("path", "serverlog");
	pChannel->setProperty("rotation", "50 M");
	pChannel->setProperty("archive", "timestamp");
	pChannel->setProperty("purgeAge", "7 days");
	pChannel->setProperty("flush", "false");
	Poco::Logger::root().setChannel(pChannel);
	return true;
}

bool setPrintLogLevel(int logLevel)
{
	if(logLevel < LEVEL_FATAL || logLevel > LEVEL_DEBUG)
	  return false;
	else
	  s_printLogLevel = logLevel;
	return true;
}

///> level打印等级
///> x 显示的等级
///> y 颜色
#define LOG_MSG(logLevel , x, y) {\
	if( s_printLogLevel <(logLevel)) return 0; \
	char buffer[8192]; \
	buffer[8191] = 0;	\
	int n = 0;		\
	set_console_color(y);	\
	try{	\
	Poco::Timestamp t;	\
	Poco::DateTime dt(t);	\
	n = snprintf(buffer, sizeof(buffer) - 1, "%02d-%02d %02d:%02d:%02d|", dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second()); \
	}	\
	catch(Poco::Exception& e){	\
	}	\
	n += snprintf(buffer + n, sizeof(buffer) - 1 - n, "%s ", x);	\
	va_list ap; \
	va_start(ap, fmt); \
	n += vsnprintf(buffer + n, sizeof(buffer) - 1 - n, fmt, ap); \
	va_end(ap);	\
	Poco::Logger::get("Logger").information(buffer); \
	n += snprintf(buffer + n, sizeof(buffer) - 1 - n, "\n"); \
	fputs(buffer, stdout); \
	reset_console_color();\
	return int(n);	\
}

int debugf(const char* fmt, ...)
{
	LOG_MSG(LEVEL_DEBUG, "debug", COLOR_DEBUG)
}

int tracef(const char* fmt, ...)
{
	LOG_MSG(LEVEL_TRACE, "trace", COLOR_TRACE)
}

int infof(const char* fmt, ...)
{
	LOG_MSG(LEVEL_INFO, "info", COLOR_INFO)
}

int warnf(const char* fmt, ...)
{
	LOG_MSG(LEVEL_WARN, "warn", COLOR_WARN)
}

int errorf(const char* fmt, ...)
{
	LOG_MSG(LEVEL_ERROR, "error", COLOR_ERROR)
}

int fatalf(const char* fmt, ...)
{
	LOG_MSG(LEVEL_FATAL, "fatal", COLOR_FATAL)
}


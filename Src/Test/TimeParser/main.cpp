#include "Poco/DateTimeParser.h"
#include "Poco/DateTime.h"
#include <stdio.h>
int main()
{
	std::string fmt = "%Y-%m-%d %H:%M:%S %Z";
	std::string timeStr = "2016-03-07 19:11:41 +0800";
	Poco::DateTime date;
	int timeZoneDiff = 0;
	Poco::DateTimeParser::parse(fmt, timeStr, date, timeZoneDiff);
	printf("timeZoneDiff %d, date timestamp :%lld\n", timeZoneDiff, date.timestamp().epochMicroseconds());
	return 0;
}


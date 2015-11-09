#include "Poco/DateTime.h"
#include "Poco/Timespan.h"
#include "Poco/Timezone.h"
#include <stdio.h>
#include <stdlib.h>
using namespace Poco;
int main(int argc, char** argv)
{
	if(argc < 3)
	{
		printf("argc < 3.\n");
		return 0;
	}
	int hour = atoi(argv[1]);
	int minute = atoi(argv[2]);
	DateTime now;
	now.makeLocal(Timezone::tzd());
	DateTime base(now.year(), now.month(), now.day(), now.hour(), now.minute());
	DateTime des(
			now.year(),
			now.month(),
			now.day(),
			hour,
			minute);
	Timespan diff = des - base;
	int diffMinutes = diff.totalMinutes();
	diffMinutes = (diffMinutes + 24 * 60)%(24 * 60);
	if(diffMinutes <= 1)
		diffMinutes += 24 * 60;
	Timespan ts(0, 0, diffMinutes, 0, 0);
	DateTime excuteTime = base + ts;
	printf("now time day:%d, dayOfWweek:%d, hour:%d, minute:%d, second:%d\n", now.day(), now.dayOfWeek(), now.hour(), now.minute(), now.second());
	printf("des time day:%d, dayOfWeek:%d, hour:%d, minute:%d, second:%d\n", des.day(), now.dayOfWeek(), des.hour(), des.minute(), des.second());
	printf("diff timespan day:%d, hour:%d, minute:%d, totalMinutes:%d\n", diff.days(), diff.hours(), diff.minutes(), diff.totalMinutes());
	printf("diff minutes:%d\n", diffMinutes);
	printf("execute time day:%d, dayOfWeek:%d, hour:%d, minute:%d, second:%d\n", excuteTime.day(), excuteTime.dayOfWeek(), excuteTime.hour(), excuteTime.minute(), excuteTime.second());
	return 0;
}


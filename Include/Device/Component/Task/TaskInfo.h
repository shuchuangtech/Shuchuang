#ifndef __DEVICE_COMPONENT_TASK_INFO_H__
#define __DEVICE_COMPONENT_TASK_INFO_H__
#include "Poco/Types.h"
#define TaskSunday		0x40		//0100 0000
#define TaskMonday		0x20		//0010 0000
#define TaskTuesday		0x10		//0001 0000
#define TaskWedsday		0x08		//0000 1000
#define TaskThursday	0x04		//0000 0100
#define TaskFriday		0x02		//0000 0010
#define TaskSaturday	0x01		//0000 0001
struct _TaskInfo
{
	Poco::Int64 id;
	int option;
	//0 close, 1 open
	int hour;
	//0-23
	int minute;
	//0-59
	int weekday;
	//bit 0 Sunday
	//bit 1 Monday
	//bit 2 Tuesday
	//bit 3 Wedsday
	//bit 4 Thursday
	//bit 5 Friday
	//bit 6 Saturday
	//bit 7 whole week
	int active;
	//is this task active
	//0 inactive, 1 active
};
typedef struct _TaskInfo TaskInfo;
#endif


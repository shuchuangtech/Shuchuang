#include <iostream>
#include "Common/PrintLog.h"
#include "Poco/Dynamic/Struct.h"
#include "Device/Component/Task/TaskInfo.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
using namespace Poco;
extern std::string g_token;
extern std::string g_uuid;
extern std::string g_buf;
extern bool sendRequest(std::string content);
extern TaskInfo** pTask;
extern int tasksNum;

void getTasks();
void addTask();
void deleteTask();
void updateTask();
void showTasksList();
void updateTasksList(const char* buf);

void showTaskOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Get tasks" << std::endl << "2.Add task" << std::endl;
		std::cout << "3.Delete task" << std::endl << "4.Update task"<< std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				getTasks();
				break;
			case 2:
				addTask();
				break;
			case 3:
				deleteTask();
				break;
			case 4:
				updateTask();
				break;
			case 0:
				return;
			default:
				break;
		}
	}

}

void getTasks()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.list";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
	updateTasksList(g_buf.c_str());
	showTasksList();
}

void addTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.add";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	DynamicStruct dsTask;
	printf("\n>>>>    Add task    <<<<\n");
	printf("Task option:");
	int option;
	std::cin >> option;
	printf("Hour:");
	int hour;
	std::cin >> hour;
	printf("Minute:");
	int minute;
	std::cin >> minute;
	printf("Active:");
	int active;
	std::cin >> active;
	printf("Weekday:");
	char str[9];
	std::cin >> str;
	unsigned short mask = 0x40;
	int weekday = 0;
	for(int i = 0; i < 7; i++)
	{
		if(str[i] == '1')
			weekday |= mask;
		mask = mask >> 1;
	}
	printf("%x\n", weekday);
	dsTask["option"] = option;
	dsTask["hour"] = hour;
	dsTask["minute"] = minute;
	dsTask["weekday"] = weekday;
	dsTask["active"] = active;
	param["task"] = dsTask;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void deleteTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	printf("Which task to be removed?\n");
	int choice;
	std::cin >> choice;
	Int64 id = pTask[choice - 1]->id;
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.remove";
	DynamicStruct task;
	task["id"] = id;
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	param["task"] = task;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void updateTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	printf("Which task to be modified?\n");
	int choice;
	std::cin >> choice;
	printf("Option:");
	int option;
	std::cin >> option;
	int hour;
	printf("Hour:");
	std::cin >> hour;
	printf("Minute:");
	int minute;
	std::cin >> minute;
	printf("Weekday:");
	char str[9];
	std::cin >> str;
	unsigned short mask = 0x40;
	int weekday = 0;
	for(int i = 0; i < 7; i++)
	{
		if(str[i] == '1')
			weekday |= mask;
		mask = mask >> 1;
	}
	printf("%x\n", weekday);
 
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.modify";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	DynamicStruct dsTask;
	dsTask["id"] = pTask[choice-1]->id;
	dsTask["option"] = option;
	dsTask["hour"] = hour;
	dsTask["minute"] = minute;
	dsTask["weekday"] = weekday;
	param["task"] = dsTask;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void showTasksList()
{
	printf("Totally %d tasks:\n", tasksNum);
	for(int i = 0; i < tasksNum; i++)
	{
		printf("Task %d:\n"
				"{\n"
				"\toption:%d\n"
				"\thour:%d\n"
				"\tminute:%d\n"
				"\tweekday:%x\n"
				"}\n", i + 1, pTask[i]->option, pTask[i]->hour, pTask[i]->minute, pTask[i]->weekday);
	}
}

void updateTasksList(const char* buf)
{
	JSON::Parser parser;
	Dynamic::Var var;
	try
	{
		var = parser.parse(buf);
	}
	catch(Exception& e)
	{
		printf("%s\n", e.message().c_str());
		return;
	}
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj.isNull())
	{
		printf("pObj is null.\n");
		return;
	}
	JSON::Object::Ptr pParam = pObj->getObject("param");
	if(pParam.isNull())
	{
		printf("pParam is null.\n");
		return;
	}
	JSON::Array::Ptr pArray = pParam->getArray("tasks");
	tasksNum = pArray->size();
	for(unsigned int i = 0; i < pArray->size(); i++)
	{
		JSON::Object::Ptr pObjTask = pArray->getObject(i);
		if(pObjTask.isNull())
		{
			printf("%u task error.\n", i);
			continue;
		}
		pTask[i]->id = pObjTask->getValue<Int64>("id");
		pTask[i]->option = pObjTask->getValue<int>("option");
		pTask[i]->hour = pObjTask->getValue<int>("hour");
		pTask[i]->minute = pObjTask->getValue<int>("minute");
		pTask[i]->weekday = pObjTask->getValue<int>("weekday");
	}
}


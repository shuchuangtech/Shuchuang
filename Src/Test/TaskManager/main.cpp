#include "Device/Component/Task/TaskManager.h"
#include "Device/Component/Task/TaskInfo.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Dynamic/Struct.h"
#include "Common/PrintLog.h"
#include "Common/ConfigManager.h"
#include <stdio.h>
CTaskManager* task_manager = NULL;
TaskInfo** pTask = NULL;
int tasksNum;
char getChoice()
{
	printf("\n>>>>    TaskManager test begin!    <<<<\n");
	printf("1. Show tasks list.\n"
			"2. Add task.\n"
			"3. Modify task.\n"
			"4. Remove task.\n"
			"0. Exit.\n"
			"enter your choice:");
	char c;
	char cret = '0';
	while((c = getchar()) != '\n')
	{
		cret = c;
	}
	return cret;
}

void updateTasksList()
{
	printf("updateTasksList()\n");
	tasksNum = task_manager->getTasksNumber();
	if(tasksNum > 0)
	{
		Poco::JSON::Object::Ptr pObj = new Poco::JSON::Object;
		task_manager->getTasks(pObj);
		Poco::DynamicStruct ds = *pObj;
		printf("ds:%s\n", ds.toString().c_str());
		if(pObj->has("tasks"))
			printf("pObj has tasks\n");
		if(pObj->isArray("tasks"))
			printf("pObj is array tasks\n");
		Poco::JSON::Array::Ptr pArray = pObj->getArray("tasks");
		printf("tasksNum:%d, array size:%d\n", tasksNum, pArray->size());
		for(int i = 0; i < tasksNum; i++)
		{
			memset(pTask[i], 0, sizeof(TaskInfo));
			Poco::Dynamic::Var var = pArray->get(i);
			Poco::DynamicStruct dss = var.extract<Poco::DynamicStruct>();
			pTask[i]->id = (Poco::Int64)dss["id"].extract<Poco::Int64>();
			pTask[i]->option = dss["option"].extract<int>();
			pTask[i]->hour = dss["hour"].extract<int>();
			pTask[i]->minute = dss["minute"].extract<int>();
			pTask[i]->weekday = dss["weekday"].extract<int>();
		}
	}
}

void showTasksList()
{
	printf("\n>>>>  Show tasks list    <<<<\n");
	if(tasksNum > 0)
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
	else
	{
		printf("Task list is empty.\n");
	}
}

void addTask()
{
	printf("\n>>>>    Add task    <<<<\n");
	printf("Task option:");
	int option;
	int ret;
	ret = 0;
	ret = scanf("%d", &option);
	printf("Hour:");
	int hour;
	ret = scanf("%d", &hour);
	printf("Minute:");
	int minute;
	ret = scanf("%d", &minute);
	printf("Weekday:");
	int weekday = 0;
	char str[9];
	ret = scanf("%s", str);
	unsigned short mask = 0x40;
	for(int i = 0; i < 7; i++)
	{
		if(str[i] == '1')
			weekday |= mask;
		mask = mask >> 1;
	}
	printf("%x\n", weekday);
	Poco::JSON::Object::Ptr pObj = new Poco::JSON::Object;
	pObj->set("option", option);
	pObj->set("hour", hour);
	pObj->set("minute", minute);
	pObj->set("weekday", weekday);
	Poco::Int64 retval = task_manager->addTask(pObj);
	printf("Add task result: %llu.\n", retval);
	updateTasksList();
	ret++;
	getchar();
}

void modifyTask()
{
	int ret;
	ret = 0;
	printf("\n>>>>    Modify task    <<<<\n");
	printf("Which task would be modified:");
	int num;
	ret = scanf("%d", &num);
	if(num > tasksNum)
	{
		printf("Num input error.\n");
	}
	else
	{
		printf("Task option:");
		int option;
		ret = scanf("%d", &option);
		printf("Hour:");
		int hour;
		ret = scanf("%d", &hour);
		printf("Minute:");
		int minute;
		ret = scanf("%d", &minute);
		printf("Weekday:");
		char str[8];
		unsigned char mask = 0x40;
		ret = scanf("%s", str);
		int weekday = 0;
		for(int i = 0; i < 7; i++)
		{
			if(str[i] == '1')
			{
				weekday &= mask;
				mask >>= 1;
			}
		}
		printf("prepare to modify Task[%llu]\n", pTask[num]->id);
		Poco::JSON::Object::Ptr pObj = new Poco::JSON::Object;
		pObj->set("id",  (Poco::Int64)pTask[num - 1]->id);
		pObj->set("option", option);
		pObj->set("hour", hour);
		pObj->set("minute", minute);
		pObj->set("weekday", weekday);
		printf("new task:(%d, %d, %d, %d)\n", option, hour, minute, weekday);
		ret = task_manager->modifyTask(pObj);
		printf("modify task return %d\n", ret);
		updateTasksList();
		ret++;
		getchar();
	}
}

void removeTask()
{
	printf("\n>>>>    Remove task    <<<<\n");
	printf("Which task would be removed:");
	int num;
	int ret;
	ret = 0;
	ret = scanf("%d", &num);
	getchar();
	if(num > tasksNum)
	{
		printf("Num input error.\n");
	}
	else
	{
		Poco::JSON::Object::Ptr pObj = new Poco::JSON::Object;
		pObj->set("id", (Poco::Int64)pTask[num - 1]->id);
		pObj->set("option", 0);
		pObj->set("hour", 0);
		pObj->set("minute", 0);
		pObj->set("weekday", 0);
		printf("prepare to remove: %llu\n", pTask[num - 1]->id);
		task_manager->removeTask(pObj);
	}
	ret++;
	updateTasksList();
}

int main(int argc, char** argv)
{
	initPrintLogger();
	CConfigManager* config = CConfigManager::instance();
	if(!config->init("./testConfig"))
	{
		printf("Config manager init failed.\n");
		return 0;
	}
	task_manager = CTaskManager::instance();
	pTask = new TaskInfo* [20];
	for(int i = 0; i < 20; i++)
	{
		pTask[i] = new TaskInfo;
	}
	updateTasksList();
	while(1)
	{
		char c = getChoice();
		switch(c)
		{
			case '1':
				showTasksList();
				break;
			case '2':
				addTask();
				break;
			case '3':
				modifyTask();
				break;
			case '4':
				removeTask();
				break;
			case '0':
				goto beforeExit;
			default:
				break;
		}
	}
beforeExit:
	task_manager->stopAllTasks();
	printf("Task manager stopAllTasks return.\n");
	for(int i = 0; i < 20; i++)
	{
		delete pTask[i];
	}
	delete[] pTask;
	printf("Delete pTask.\n");
	return 0;
}


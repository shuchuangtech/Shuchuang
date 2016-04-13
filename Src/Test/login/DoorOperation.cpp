#include "Common/PrintLog.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include <iostream>
#include "Poco/Dynamic/Struct.h"
using namespace Poco;
extern bool sendRequest(std::string content);
extern std::string g_buf;
extern std::string g_uuid;
extern std::string g_token;
void checkDoor()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.check";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void openDoor()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.open";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void closeDoor()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.close";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void getMode()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.getmode";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}

}

void changeMode()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.chmode";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	std::cout << "Mode:";
	int mode;
	std::cin >> mode;
	param["mode"] = mode;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}

}

void showDoorOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Check" << std::endl << "2.Open" << std::endl;
		std::cout << "3.Close" << std::endl << "4.Get mode" << std::endl << "5.Change mode" << std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				checkDoor();
				break;
			case 2:
				openDoor();
				break;
			case 3:
				closeDoor();
				break;
			case 4:
				getMode();
				break;
			case 5:
				changeMode();
				break;
			case 0:
				return;
			default:
				break;
		}
	}

}



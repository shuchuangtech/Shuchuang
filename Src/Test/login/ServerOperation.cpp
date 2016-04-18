#include "Common/PrintLog.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include <iostream>
#include "Poco/Dynamic/Struct.h"
using namespace Poco;
extern bool sendRequest(std::string content);
extern std::string g_buf;

void checkDevice()
{
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.check";
	DynamicStruct param;
	std::string uuid;
	std::cout << "uuid:";
	std::cin >> uuid;
	param["uuid"] = uuid;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}

}

void checkUpdate()
{
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "update.check";
	DynamicStruct param;
	std::string type;
	std::cout << "device type:";
	std::cin >> type;
	param["type"] = type;
	param["uuid"] = "any";
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void showServerOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Check Device State" << std::endl << "2.Check Update" << std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				checkDevice();
				break;
			case 2:
				checkUpdate();
				break;
			case 0:
				return;
			default:
				break;
		}
	}

}


#include <iostream>
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Struct.h"
#include "Common/PrintLog.h"
using namespace Poco;
extern std::string g_token;
extern std::string g_buf;
extern std::string g_uuid;
extern bool sendRequest(std::string content);
void resetSystem()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "system.reset";
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

void showSystemOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Reset" << std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				resetSystem();
				break;
			case 0:
				return;
			default:
				break;
		}
	}
}



#include "Common/PrintLog.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include <iostream>
#include "Poco/Dynamic/Struct.h"
using namespace Poco;
extern bool sendRequest(std::string content);
void checkUpdate()
{
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "update.check";
	DynamicStruct param;
	param["type"] = "sc-lock01";
	param["uuid"] = "any";
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}

}

void showUpdateOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Check" << std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				checkUpdate();
				break;
			case 0:
				return;
			default:
				break;
		}
	}

}


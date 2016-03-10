#include "Common/ConfigManager.h"
#include "Common/PrintLog.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/VarHolder.h"
#include <stdio.h>
#include <iostream>
using namespace Poco;
int main()
{
	initPrintLogger("./testlog");
	CConfigManager* config = CConfigManager::instance();
	std::cout << "Config Editor is running." << std::endl;
	std::cout << "Input config file path:";
	std::string path = "";
	std::cin >> path;
	config->init(path);
	JSON::Object::Ptr pConfig;
	config->getAllConfig(pConfig);
	DynamicStruct ds = *pConfig;
	std::cout << ds.toString().c_str() << std::endl;
	std::cout << "1.Edit exists config\n2.Add new config" << std::endl;
	int choice;
	std::cin >> choice;
	if(choice == 1)
	{
		std::cout << "Input config name:";
		std::string configName = "";
		std::cin >> configName;
	}
	else if(choice == 2)
	{
		std::cout << "Input config name:";
		std::string configName = "";
		std::cin >> configName;
		std::cout << "Add config, end with key name 'end'" << std::endl;
		std::string confKey = "";
		std::cout << "key:";
		std::cin >> confKey;
		JSON::Object::Ptr pConfObj = new JSON::Object;
		while(confKey != "end")
		{
			std::cout << "Value type:1.string, 2.int, 3.array";
			std::cin >> choice;
			std::cout << "value:";
			switch(choice)
			{
				case 1:
				{
					std::string confValue = "";
					std::cin >> confValue;
					pConfObj->set(confKey, confValue);
					break;
				}
				case 2:
				{
					int confValue = 0;
					std::cin >> confValue;
					pConfObj->set(confKey, confValue);
					break;
				}
			}
			std::cout << "key:";
			std::cin >> confKey;
		}
		config->setConfig(configName, pConfObj);
	}
	else
	{
	}

	return 0;
}


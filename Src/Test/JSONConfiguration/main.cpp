#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/VarHolder.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/AutoPtr.h"
#include "Poco/FileStream.h"
#include <stdio.h>
#include <vector>
using namespace Poco;
void initDefaultConfig(JSON::Object::Ptr& pRoot, int choice)
{
	JSON::Object::Ptr pNode = new JSON::Object;
	if(choice == 1)
	{
		//DeviceInfo
		JSON::Object::Ptr pDeviceInfo = new JSON::Object;
		pDeviceInfo->set("uuid", "SC0000000001");
		pDeviceInfo->set("type", "sc-lock0001");
		pDeviceInfo->set("name", "SC0000000001");
		pDeviceInfo->set("manufacture", "Shuchuangtech");
		//RegProxy
		JSON::Object::Ptr pRegProxy = new JSON::Object;
		pRegProxy->set("host", "shuchuangtech.com");
		pRegProxy->set("ssl_port", 12222);
		pRegProxy->set("reg_port", 13333);
		//Tasks
		JSON::Array::Ptr pTasks = new JSON::Array;
		//DataPath
		JSON::Object::Ptr pDataPath = new JSON::Object;
		printf("Database on 1.board\t2.virtual machine\n");
		int i = 0;
		int ret = scanf("%d", &i);
		if(ret && i == 1)
		{
			pDataPath->set("User", "/mnt/nand1-1/Application/user.db");
			pDataPath->set("Operation", "/mnt/nand1-1/Application/oprecord.db");
		}
		else if(ret && i == 2)
		{
			pDataPath->set("User", "/home/hj/Dev_Env/Shuchuang/user.db");
			pDataPath->set("Operation", "/home/hj/Dev_Env/Shuchuang/oprecord.db");
		}
		//JSON::Object::Ptr nil = NULL;
		//pTasks->add(nil);
		pNode->set("Tasks", pTasks);
		pNode->set("DeviceInfo", pDeviceInfo);
		pNode->set("RegProxy", pRegProxy);
		pNode->set("DataPath", pDataPath);
	}
	else if(choice == 2)
	{
		JSON::Object::Ptr pRegServer = new JSON::Object;
		pRegServer->set("ssl_port", 12222);
		pRegServer->set("reg_port", 13333);

		JSON::Object::Ptr pHTTPSAcceptor = new JSON::Object;
		pHTTPSAcceptor->set("port", 9888);
		pHTTPSAcceptor->set("cert", "./cert.pem");
		pHTTPSAcceptor->set("privkey", "privkey.pem");

		JSON::Object::Ptr pHTTPServer = new JSON::Object;
		pHTTPServer->set("port", 8777);

		pNode->set("RegServer", pRegServer);
		pNode->set("HTTPSAcceptor", pHTTPSAcceptor);
		pNode->set("HTTPServer", pHTTPServer);
	}
	pRoot->set("root", pNode);
}

int main()
{
	int choice = 0;
	printf("1. Generate device default config.\n"
			"2. Generate server default config.\n");
	int ret = scanf("%d", &choice);
	if(ret < 0)
		return 0;
	FileOutputStream confFile("./test.conf");
	JSON::Object::Ptr pObj = new JSON::Object;
	initDefaultConfig(pObj, choice);
	DynamicStruct ds = *pObj;
	printf("%s\n", ds.toString().c_str());
	AutoPtr<Util::JSONConfiguration> conf = new Util::JSONConfiguration(pObj);
	conf->save(confFile);
	return 0;
}


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
		pDeviceInfo->set("mac", "1234567890ab");
		//RegProxy
		JSON::Object::Ptr pRegProxy = new JSON::Object;
		pRegProxy->set("host", "shuchuangtech.com");
		pRegProxy->set("ssl_port", 12222);
		pRegProxy->set("reg_port", 13333);
		//APNS
		JSON::Object::Ptr pAPNS = new JSON::Object;
		pAPNS->set("MobileToken", "");
		//Tasks
		JSON::Array::Ptr pTasks = new JSON::Array;
		//DataPath
		JSON::Object::Ptr pDataPath = new JSON::Object;
		printf("Database on 1.board\t2.virtual machine\n");
		int i = 0;
		int ret = scanf("%d", &i);
		if(ret && i == 1)
		{
			pDataPath->set("User", "/mnt/nand1-2/Application/user.db");
			pDataPath->set("Operation", "/mnt/nand1-2/Application/oprecord.db");
		}
		else if(ret && i == 2)
		{
			pDataPath->set("User", "/home/huang_jian/Dev_Env/Shuchuang/Config/test1/user.db");
			pDataPath->set("Operation", "/home/huang_jian/Dev_Env/Shuchuang/Config/test1/oprecord.db");
		}
		//JSON::Object::Ptr nil = NULL;
		JSON::Object::Ptr pReset = new JSON::Object;
		if(ret && i == 1)
		{
			pReset->set("UserDB", "/mnt/nand1-2/Application/backup/user.db");
			pReset->set("Config", "/mnt/nand1-2/Application/backup/global.conf");
		}
		else if(ret && i == 2)
		{
			pReset->set("UserDB", "/home/huang_jian/Dev_Env/Shuchuang/Config/test1/backup/user.db");
			pReset->set("Config", "/home/huang_jian/Dev_Env/Shuchuang/backup/Config/test1/global.conf");
		}
		//Update
		JSON::Object::Ptr pUpdate = new JSON::Object;
		pUpdate->set("server", "shuchuangtech.com");
		pUpdate->set("username", "sc");
		pUpdate->set("password", "SCDevicePublic123");
		if(ret && i == 1)
		{
			pUpdate->set("appPath", "/mnt/nand1-2/Application/DeviceTest");
			pUpdate->set("infoPath", "/mnt/nand1-2/Application/Version.info");
		}
		else if(ret && i == 2)
		{
			pUpdate->set("appPath", "/home/huang_jian/Dev_Env/Shuchuang/DeviceTest");
			pUpdate->set("infoPath", "/home/huang_jian/Dev_Env/Shuchuang/Version.info");
		}

		//pTasks->add(nil);
		pNode->set("APNS", pAPNS);
		pNode->set("Tasks", pTasks);
		pNode->set("DeviceInfo", pDeviceInfo);
		pNode->set("RegProxy", pRegProxy);
		pNode->set("DataPath", pDataPath);
		pNode->set("Reset", pReset);
		pNode->set("Update", pUpdate);
	}
else if(choice == 2)
	{
		JSON::Object::Ptr pRegServer = new JSON::Object;
		pRegServer->set("ssl_port", 12222);
		pRegServer->set("reg_port", 13333);

		JSON::Object::Ptr pHTTPServer = new JSON::Object;
		pHTTPServer->set("port", 8777);
		pHTTPServer->set("port", 9888);
		pHTTPServer->set("cert", "./cert.pem");
		pHTTPServer->set("privkey", "./privkey.pem");
		
		JSON::Object::Ptr pUpdate = new JSON::Object;
		pUpdate->set("DirPath", "./update/");
		
		JSON::Object::Ptr pAPNS = new JSON::Object;
		JSON::Array::Ptr pProxy = new JSON::Array;
		pProxy->add("bmob");
		pAPNS->set("Proxy", pProxy);
		JSON::Object::Ptr pBmob = new JSON::Object;
		pBmob->set("Host", "api.bmob.cn");
		pBmob->set("URI", "/1/push");
		pBmob->set("APPID", "27f1f3599a223cfa40bb5c5e5daedd7a");
		pBmob->set("APPKey", "0952518ed9dd101fab4c5c02d957f62d");
		pAPNS->set("bmob", pBmob);

		JSON::Object::Ptr pMode = new JSON::Object;
		pMode->set("mode", 0);

		pNode->set("RegServer", pRegServer);
		pNode->set("HTTPServer", pHTTPServer);
		pNode->set("Update", pUpdate);
		pNode->set("APNS", pAPNS);
		pNode->set("UserMode", pMode);
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


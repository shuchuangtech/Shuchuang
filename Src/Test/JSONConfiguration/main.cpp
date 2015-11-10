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
void initDefaultConfig(JSON::Object::Ptr& pRoot)
{
	JSON::Object::Ptr pDeviceInfo = new JSON::Object;
	pDeviceInfo->set("uuid", "SC0000000001");
	pDeviceInfo->set("type", "sc-lock0001");
	pDeviceInfo->set("name", "SC0000000001");
	pDeviceInfo->set("manufacture", "Shuchuangtech");

	JSON::Object::Ptr pRegProxy = new JSON::Object;
	pRegProxy->set("host", "shuchuangtech.com");
	pRegProxy->set("ssl_port", 12222);
	pRegProxy->set("reg_port", 13333);

	JSON::Array::Ptr pTasks = new JSON::Array;
	//JSON::Object::Ptr nil = NULL;
	//pTasks->add(nil);
	JSON::Object::Ptr pNode = new JSON::Object;
	pNode->set("Tasks", pTasks);
	pNode->set("DeviceInfo", pDeviceInfo);
	pNode->set("RegProxy", pRegProxy);

	pRoot->set("root", pNode);
}

int main()
{
	FileOutputStream confFile("./test.conf");
	JSON::Object::Ptr pObj = new JSON::Object;
	initDefaultConfig(pObj);
	DynamicStruct ds = *pObj;
	printf("%s\n", ds.toString().c_str());
	AutoPtr<Util::JSONConfiguration> conf = new Util::JSONConfiguration(pObj);
	conf->save(confFile);
	return 0;
}


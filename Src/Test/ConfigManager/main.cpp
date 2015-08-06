#include "Common/ConfigManager.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Struct.h"
#include <stdio.h>
int main()
{
	CConfigManager* config = CConfigManager::instance();
	config->init("./Config");
	DynamicStruct attr3;
	attr3["a3a1"] = "ccc";
	attr3["a3a2"] = "ddd";
	Poco::JSON::Object::Ptr pObj = new JSON::Object;
	pObj->set("attr1", "aaa");
	pObj->set("attr2", "bbb");
	pObj->set("attr3", attr3);
	config->setConfig("global", pObj);

	DynamicStruct attr4;
	attr4["d1"] = "d1";
	attr4["d2"] = 22;
	DynamicStruct attr5;
	attr5["e1"] = true;
	attr4["d3"] = attr5;



	Poco::JSON::Object::Ptr pObj1;
	pObj1 = new JSON::Object;
	pObj1->set("c1", attr3);
	pObj1->set("c2", attr4);

	config->setConfig("Server", pObj1);

	Poco::JSON::Object::Ptr pObj2;
	config->getConfig("global", pObj2);
	DynamicStruct ds = *pObj2;
	printf("%s\n", ds.toString().c_str());
	return 0;
}


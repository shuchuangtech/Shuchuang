#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/VarHolder.h"
#include "Poco/AutoPtr.h"
#include "Poco/FileStream.h"
#include <stdio.h>
#include <vector>
using namespace Poco;
int main()
{
	
	JSON::Object::Ptr obj = new JSON::Object;
	DynamicStruct ds;
	ds["attr1"] = "aaaa";
	ds["attr2"] = "bbbb";
	DynamicStruct param;
	param["name"] = "huangjian";
	param["age"] = 18;
	ds["attr3"] = param;
	JSON::Array array;
	array.set(0u, "123");
	array.set(1u,"321");
	ds["attr4"] = array;
	obj->set("Server", ds);
	AutoPtr<Util::JSONConfiguration> conf = new Util::JSONConfiguration(obj);
	FileOutputStream confFile("./test.conf");
	conf->save(confFile);
	std::string str = conf->getRawString("Server");
	printf("%s\n", str.c_str());
	AutoPtr<Util::JSONConfiguration> conf2 = new Util::JSONConfiguration;
	memcpy(conf2, conf, sizeof(Util::JSONConfiguration));
	std::string str2 = conf2->getRawString("Server");
	printf("%d\n", sizeof(Util::JSONConfiguration));
	printf("%s\n", str2.c_str());
	return 0;
}


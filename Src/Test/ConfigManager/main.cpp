#include "Common/ConfigManager.h"
#include "Common/PrintLog.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/VarHolder.h"
#include <stdio.h>
using namespace Poco;
int main()
{
	/*
	JSON::Object::Ptr pObj = new JSON::Object;
	JSON::Array::Ptr pArr = new JSON::Array;
	DynamicStruct attr30;
	attr30["a1"] = 123;
	attr30["a2"] = "abc";
	DynamicStruct attr31;
	attr31["a3"] = 345;
	attr31["a4"] = "cba";

	pArr->add(attr30);
	pArr->add(attr31);
	
	pObj->set("array", pArr);
	DynamicStruct ds = *pObj;
	printf("%s\n", ds.toString().c_str());

	Dynamic::Array arr = *pArr;
	for(unsigned int i = 0; i < arr.size(); i++)
	{
		Dynamic::Var var = arr[i];
		printf("arr[%d]: %s\n", i, var.toString().c_str());
	}
	*/
	initPrintLogger();
	CConfigManager* config = CConfigManager::instance();
	config->init("./testConfig");
	
	Poco::JSON::Object::Ptr pObj = new JSON::Object;
	pObj->set("attr1", "aaa");
	DynamicStruct attr2;
	attr2["attr21"] = "abc";
	attr2["attr22"] = "cba";
	pObj->set("attr2", attr2);
	JSON::Array::Ptr pArr = new JSON::Array;
	DynamicStruct attr30;
	attr30["a1"] = 123;
	attr30["a2"] = "abc";
	DynamicStruct attr31;
	attr31["a3"] = 345;
	attr31["a4"] = "cba";
	pArr->add(attr30);
	pArr->add(attr31);
	JSON::Object::Ptr pConfig = new JSON::Object;
	pConfig->set("array", pArr);
	pConfig->set("object", pObj);
	DynamicStruct ds = *pConfig;
	config->setConfig("array", pArr);
	config->setConfig("array2", pArr);
	config->setConfig("object", pObj);
	config->setConfig("object2", pObj);
	/*
	JSON::Object::Ptr pObj = new JSON::Object;
	config->getConfig("object", pObj);
	DynamicStruct ds_obj = *pObj;
	printf("object:\n%s\n", ds_obj.toString().c_str());
	JSON::Array::Ptr pArr = new JSON::Array;
	config->getConfig("array", pArr);
	Dynamic::Array arr = *pArr;
	for(unsigned int i = 0; i < arr.size(); i++)
	{
		Dynamic::Var var = arr[i];
		printf("array[%d]:%s\n", i, var.toString().c_str());
	}
	*/
	return 0;
}


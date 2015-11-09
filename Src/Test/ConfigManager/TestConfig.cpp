#include "TestConfig.h"
#include "Poco/Dynamic/Struct.h"
#include <stdio.h>
TestConfig::TestConfig()
{
	m_config = new Poco::JSON::Object;
}

TestConfig::~TestConfig()
{
}

bool TestConfig::setConfig(std::string configName, Poco::JSON::Object::Ptr& config)
{
	Poco::DynamicStruct ds0 = *config;
	printf("00000000 %s\n", ds0.toString().c_str());
	Poco::JSON::Object::Ptr pObj = new Poco::JSON::Object;
	//Poco::DynamicStruct ds = *pObj;
	//printf("11111111 %s\n", ds.toString().c_str());
	pObj->set("attr1", "abc");
	Poco::DynamicStruct ds2 = *pObj;
	printf("22222222 %s\n", ds2.toString().c_str());
	m_config->set(configName, config);
	Poco::DynamicStruct ds3 = *m_config;
	printf("set object: %s\n", ds3.toString().c_str());
	return true;
}

bool TestConfig::setConfig(std::string configName, Poco::JSON::Array::Ptr& config)
{
	m_config->set(configName, config);
	Poco::DynamicStruct ds = *m_config;
	printf("set array: %s\n", ds.toString().c_str());
	return true;
}


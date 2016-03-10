#ifndef __DEVICE_COMPONENT_SYSTEM_MANAGER_H__
#define __DEVICE_COMPONENT_SYSTEM_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
class CSystemManager
{
public:
	CSystemManager();
	~CSystemManager();
	static CSystemManager* instance()
	{
		static Poco::SingletonHolder<CSystemManager> sh;
		return sh.get();
	}
	//user interface
	bool resetConfig(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool startUpdate(Poco::JSON::Object::Ptr& param, std::string& detail);
	//internal interface
	bool synchronizeTime();
};
#endif


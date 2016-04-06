#ifndef __COMMON_CONFIG_MANAGER_H__
#define __COMMON_CONFIG_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Mutex.h"
class CConfigManager
{
public:
	static CConfigManager* instance()
	{
		static Poco::SingletonHolder<CConfigManager> sh;
		return sh.get();
	}
	CConfigManager();
	~CConfigManager();
	bool init(const std::string path);
	bool resetConfig();
	bool getAllConfig(Poco::JSON::Object::Ptr& config);
	bool getConfig(const std::string configName, Poco::JSON::Object::Ptr& config);
	bool getConfig(const std::string configName, Poco::JSON::Array::Ptr& config);
	bool setConfig(const std::string configName, Poco::JSON::Object::Ptr config);
	bool setConfig(const std::string configName, Poco::JSON::Array::Ptr config);
private:
	std::string						m_path;
	Poco::Mutex						m_mutex;
};
#endif


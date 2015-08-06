#ifndef __COMMON_CONFIG_MANAGER_H__
#define __COMMON_CONFIG_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/Mutex.h"
using namespace Poco;
class CConfigManager
{
public:
	static CConfigManager* instance()
	{
		static SingletonHolder<CConfigManager> sh;
		return sh.get();
	}
	CConfigManager();
	~CConfigManager();
	bool init(const std::string path);
	bool getConfig(const std::string configName, JSON::Object::Ptr& config);
	bool setConfig(const std::string configName, JSON::Object::Ptr config);
private:
	AutoPtr<Util::JSONConfiguration>	m_config;
	std::string		m_path;
	Mutex			m_mutex;
};
#endif


#ifndef __DEVICE_COMPONENT_SYSTEM_MANAGER_H__
#define __DEVICE_COMPONENT_SYSTEM_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/Timer.h"
#include "Poco/Net/NTPEventArgs.h"
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
	bool getDevVersion(Poco::JSON::Object::Ptr& param, std::string& detail);
	void rebootSystem();
	//internal interface
	bool synchronizeTime();
	std::string getFileMD5(const std::string& filePath);
	//timerHandler
	void onNTPEvent(const void* pSender, Poco::Net::NTPEventArgs& arg);
	void handleUpdate(Poco::Timer& timer);
private:
	Poco::Timer* m_timer;
	bool m_updating;
	std::string m_update_version;
};
#endif


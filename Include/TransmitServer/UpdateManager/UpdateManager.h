#ifndef __SERVER_UPDATE_MANAGER_H__
#define __SERVER_UPDATE_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/Path.h"
class CUpdateManager
{
public:
	static CUpdateManager* instance()
	{
		static Poco::SingletonHolder<CUpdateManager> sh;
		return sh.get();
	}
	CUpdateManager();
	~CUpdateManager();
	bool init();
	bool checkUpdate(Poco::JSON::Object::Ptr& param, std::string& detail);
private:
	Poco::Path	m_update_path;
};
#endif


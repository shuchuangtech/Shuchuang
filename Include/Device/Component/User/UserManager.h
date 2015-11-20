#ifndef __COMPONENT_USER_MANAGER_H__
#define __COMPONENT_USER_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "Device/Util/UserRecord.h"
class CUserManager
{
public:
	CUserManager();
	~CUserManager();
	static CUserManager* instance()
	{
		static Poco::SingletonHolder<CUserManager> sh;
		return sh.get();
	}
	bool init(const std::string&);
	bool verifyUser(std::string token);
	bool login(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool passwd(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool logout(Poco::JSON::Object::Ptr& param, std::string& detail);
private:
	bool generateNewMD5String(std::string& md5str);
	bool checkUserValidity(UserRecordNode& user);
	bool verifyUserPassword(const std::string&, const std::string&, const std::string&, const std::string&);
	CUserRecord*								m_user_record;
	//username, challenge
	std::map<std::string, std::string>			m_challenge_map;
	Poco::Mutex									m_map_mutex;
};
#endif


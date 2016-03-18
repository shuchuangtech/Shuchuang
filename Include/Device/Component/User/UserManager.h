#ifndef __COMPONENT_USER_MANAGER_H__
#define __COMPONENT_USER_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "Device/Util/UserRecord.h"
#include "Poco/Timer.h"
class CUserManager
{
public:
	enum {
		USER_AUTHORITY_ADMIN = 9,
		USER_AUTHORITY_LONG = 8,
		USER_AUTHORITY_TEMP = 7
	};
	CUserManager();
	~CUserManager();
	static CUserManager* instance()
	{
		static Poco::SingletonHolder<CUserManager> sh;
		return sh.get();
	}
	bool init(const std::string&);
	bool start();
	bool stop();
	bool verifyUser(std::string token);
	bool login(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool passwd(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool logout(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool addUser(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool deleteUser(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool topUpUser(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool listUser(Poco::JSON::Object::Ptr& param, std::string& detail);
	//internal
	bool	getUserNameFromToken(const std::string& token, std::string& username);
	int		userAuthority(const std::string& token);
	bool	userOpenDoor(const std::string& token);
	bool	canUserOpenDoor(const std::string& token);
	void	timerCallback(Poco::Timer& timer);
private:
	bool generateNewMD5String(std::string& md5str);
	bool checkUserValidity(UserRecordNode& user);
	bool verifyUserPassword(const std::string&, const std::string&, const std::string&, const std::string&);
	CUserRecord*								m_user_record;
	//username, challenge
	std::map<std::string, std::string>			m_challenge_map;
	std::map<std::string, int>				m_cache_map;
	Poco::Mutex									m_challenge_mutex;
	Poco::Mutex									m_cache_mutex;
	Poco::Timer									m_timer;
	bool										m_started;
};
#endif


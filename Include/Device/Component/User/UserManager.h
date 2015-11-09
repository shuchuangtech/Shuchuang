#ifndef __COMPONENT_USER_MANAGER_H__
#define __COMPONENT_USER_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/JSON/Object.h"
#include "Poco/ThreadPool.h"
#include "Poco/Mutex.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/Timestamp.h"
#include "Device/Component/User/UserClient.h"
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
	bool init();
	bool login(Poco::JSON::Object::Ptr param, std::string& detail);
	bool passwd(Poco::JSON::Object::Ptr param, std::string& detail);
	bool logout(Poco::JSON::Object::Ptr param);
private:
	bool generateNewToken(std::string& token);
	CUserClient* checkClient(std::string username, std::string& token, bool create);
	void updatePasswd();
	//token, userclient
	std::map<std::string, CUserClient*>			m_client_map;
	//username, password
	std::map<std::string, std::string>			m_user_map;
	Poco::Mutex									m_map_mutex;
	Poco::Timestamp::TimeDiff					m_token_valid_period;
	Poco::UUIDGenerator							m_uuid_gen;
};
#endif


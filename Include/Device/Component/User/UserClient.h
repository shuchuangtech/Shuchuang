#ifndef __COMPONENT_USER_CLIENT_H__
#define __COMPONENT_USER_CLIENT_H__
#include "Poco/JSON/Object.h"
#include "Poco/Timestamp.h"
class CUserClient
{
public:
	enum{
		UNAUTHORIZED,
		AUTHORIZED
	};
	CUserClient(std::string username, std::string password, std::string token);
	bool login(Poco::JSON::Object::Ptr param, std::string& detail);
	bool passwd(Poco::JSON::Object::Ptr param, std::string& detail);
	bool logout(Poco::JSON::Object::Ptr param);
	bool authorized();
	bool getPasswd(std::string& passwd);
private:
	std::string		m_challenge;
	std::string		m_username;
	std::string		m_password;
	std::string		m_token;
	int				m_state;
	Poco::Timestamp		m_keepalive;
};
#endif

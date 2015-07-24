#ifndef __COMPONENT_USER_CLIENT_H__
#define __COMPONENT_USER_CLIENT_H__
#include "Poco/JSON/Object.h"
using namespace Poco;
class CUserClient
{
public:
	enum{
		UNAUTHORIZED,
		AUTHORIZED
	};
	CUserClient(std::string username, std::string password, std::string token);
	bool login(JSON::Object::Ptr param, std::string& detail);
	bool passwd(JSON::Object::Ptr param, std::string& detail);
	bool logout(JSON::Object::Ptr param);
	bool authorized();
	bool getPasswd(std::string& passwd);
private:
	std::string		m_challenge;
	std::string		m_username;
	std::string		m_password;
	std::string		m_token;
	int				m_state;
	Timestamp		m_keepalive;
};
#endif

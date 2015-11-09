#include "Device/Component/User/UserManager.h"
#include "Common/RPCDef.h"
#include "Common/PrintLog.h"
#include "Poco/FileStream.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/MD5Engine.h"
#define USER_INFO_PATH "./user.info"
using namespace Poco;
CUserManager::CUserManager()
{
	m_client_map.clear();
	m_user_map.clear();
	m_token_valid_period = 12 * 60 * 60 * 1000;
}

CUserManager::~CUserManager()
{
}

bool CUserManager::init()
{
	FileStream file(USER_INFO_PATH);
	char line[256];
	char username[32];
	char password[128];
	while(file.peek() != EOF)
	{
		file.getline(line, sizeof(line));
		sscanf(line, "%s %s", username, password);
		tracef("%s, %d: User loaded:%s, %s.\n", __FILE__, __LINE__, username, password);
		m_user_map.insert(std::make_pair<std::string, std::string>(username, password));
	}
	return true;
}

bool CUserManager::generateNewToken(std::string& token)
{
	UUID t = m_uuid_gen.create();
	MD5Engine md5;
	md5.update(t.toString());
	const DigestEngine::Digest& digest = md5.digest();
	token = DigestEngine::digestToHex(digest);
	return true;
}

CUserClient* CUserManager::checkClient(std::string username, std::string& token, bool create)
{
	Mutex::ScopedLock lock(m_map_mutex);
	std::map<std::string, CUserClient*>::iterator it_client = m_client_map.find(token);
	if(it_client != m_client_map.end())
	{
		return it_client->second;
	}
	else
	{
		if(!create)
		  return NULL;
		std::map<std::string, std::string>::iterator it_user = m_user_map.find(username);
		if(it_user == m_user_map.end())
		{
			warnf("%s, %d: User[%s] not existed.\n", __FILE__, __LINE__, username.c_str());
			return NULL;
		}
		generateNewToken(token);
		std::string password = it_user->second;
		CUserClient* user = new CUserClient(username, password, token);
		m_map_mutex.lock();
		m_client_map.insert(std::make_pair<std::string, CUserClient*>(token, user));
		m_map_mutex.unlock();
		return user;
	}
}

bool CUserManager::login(JSON::Object::Ptr pParam, std::string& detail)
{
	DynamicStruct param = *pParam;
	std::string token = "";
	if(param.contains(PARAM_TOKEN_STR))
	{
		token = param[PARAM_TOKEN_STR].toString();
	}
	std::string username = "";
	if(param.contains(PARAM_USERNAME_STR))
	{
		username = param[PARAM_USERNAME_STR].toString();
	}
	CUserClient* user = checkClient(username, token, true);
	if(user != NULL)
	{
		pParam->set(PARAM_TOKEN_STR, token);
		if(!user->login(pParam, detail))
		{
			delete user;
			m_map_mutex.lock();
			m_client_map.erase(token);
			m_map_mutex.unlock();
			pParam->remove(PARAM_TOKEN_STR);
			return false;
		}
		return true;
	}
	else
	{
		detail = "User not existed";
		return false;
	}
	return false;
}

bool CUserManager::passwd(JSON::Object::Ptr pParam, std::string& detail)
{
	DynamicStruct param = *pParam;
	std::string token = "";
	if(param.contains(PARAM_TOKEN_STR))
	{
		token = param[PARAM_TOKEN_STR].toString();
	}
	tracef("%s, %d: token %s\n", __FILE__, __LINE__, token.c_str());
	std::string username = "";
	if(param.contains(PARAM_USERNAME_STR))
	{
		username = param[PARAM_USERNAME_STR].toString();
	}
	else
	{
		detail = "Param error";
		return false;
	}
	CUserClient* user = checkClient(username, token, false);
	if(user != NULL)
	{
		pParam->set(PARAM_TOKEN_STR, token);
		if(!user->passwd(pParam, detail))
		{
			return false;
		}
		std::string passwd = "";
		user->getPasswd(passwd);
		tracef("%s, %d: User[%s] password has changed\n", __FILE__, __LINE__, username.c_str());
		std::map<std::string, std::string>::iterator it = m_user_map.find(username);
		if(it != m_user_map.end())
		{
			it->second = passwd;
		}
		updatePasswd();
		return true;
	}
	else
	{
		detail = "Token not match";
		return false;
	}
	return false;
}

bool CUserManager::logout(JSON::Object::Ptr pParam)
{
	DynamicStruct param = *pParam;
	std::string token = "";
	if(param.contains(PARAM_TOKEN_STR))
	{
		token = param[PARAM_TOKEN_STR].toString();
	}
	std::string username = "";
	if(param.contains(PARAM_USERNAME_STR))
	{
		username = param[PARAM_USERNAME_STR].toString();
	}
	CUserClient* user = checkClient(username, token, false);
	if(user)
	{
		if(user->logout(pParam))
		{
			delete user;
			m_map_mutex.lock();
			m_client_map.erase(username);
			m_map_mutex.unlock();
			return true;
		}
		return false;
	}
	return false;
}

void CUserManager::updatePasswd()
{
	m_map_mutex.lock();
	std::map<std::string, std::string>::iterator it;
	for(it = m_user_map.begin(); it != m_user_map.end(); it++)
	{
		std::string user = it->first;
		std::string password = it->second;
		FileStream file(USER_INFO_PATH, std::ios::out|std::ios::trunc);
		file<< it->first << " " << it->second << std::endl;
		tracef("%s, %d: write user[%s:%s]\n", __FILE__, __LINE__, it->first.c_str(), it->second.c_str());
	}
	m_map_mutex.unlock();
}


#include "Device/Component/User/UserClient.h"
#include "Poco/MD5Engine.h"
#include "Poco/UUIDGenerator.h"
#include "Common/RPCDef.h"
#include "Common/PrintLog.h"
#include "Poco/Dynamic/Var.h"
using namespace Poco;
CUserClient::CUserClient(std::string username, std::string password, std::string token)
{
	m_token = token;
	m_username = username;
	m_password = password;
	m_challenge = "";
	m_state = UNAUTHORIZED;
}

bool CUserClient::login(JSON::Object::Ptr pParam, std::string& detail)
{
	DynamicStruct param = *pParam;
	if(m_state != AUTHORIZED)
	{
		if(m_challenge.empty() && !param.contains(PARAM_PASSWORD_STR))
		  //first login
		{
			UUIDGenerator gen;
			UUID uuid = gen.create();
			MD5Engine md5;
			md5.update(uuid.toString());
			const DigestEngine::Digest& digest = md5.digest();
			m_challenge = DigestEngine::digestToHex(digest);
			pParam->set(PARAM_CHALLENGE_STR, m_challenge);
			tracef("%s, %d: User[%s] first login, challenge is %s.", __FILE__, __LINE__, m_username.c_str(), m_challenge.c_str());
			Timestamp t;
			m_keepalive = t;
			return true;
		}
		if(!m_challenge.empty() && param.contains(PARAM_PASSWORD_STR))
		//second login
		{
			tracef("%s, %d: m_challenge %s", __FILE__, __LINE__, m_challenge.c_str());
			std::string password = param[PARAM_PASSWORD_STR].toString();
			MD5Engine md5;
			std::string loginPassword = USER_METHOD_LOGIN + m_password;
			tracef("%s, %d: m_password %s", __FILE__, __LINE__, m_password.c_str());
			md5.update(loginPassword);
			const DigestEngine::Digest& digestLogin = md5.digest();
			std::string md5login(DigestEngine::digestToHex(digestLogin));
			tracef("%s, %d: loginpass %s", __FILE__, __LINE__, md5login.c_str());
			std::string challengePassword = md5login + m_challenge;
			md5.reset();
			md5.update(challengePassword);
			const DigestEngine::Digest& digest = md5.digest();
			std::string md5password(DigestEngine::digestToHex(digest));
			pParam->remove(PARAM_PASSWORD_STR);
			if(password == md5password)
			{
				m_challenge = "";
				m_state = AUTHORIZED;
				Timestamp t;
				m_keepalive = t;
				tracef("%s, %d: User[%s] login successfully.", __FILE__, __LINE__, m_username.c_str());
				return true;
			}
			else
			{
				warnf("%s, %d: User[%s] login verify failed.", __FILE__, __LINE__, m_username.c_str());
				m_challenge = "";
				detail = "Verify failed";
				return false;
			}
		}
		warnf("%s, %d: Unkown login request.", __FILE__, __LINE__);
		detail = "Unkown login request";
		return false;
	}
	else
	{
		Timestamp t;
		m_keepalive = t;
		return true;
	}
}

bool CUserClient::passwd(JSON::Object::Ptr pParam, std::string& detail)
{
	DynamicStruct param = *pParam;
	if(m_state == AUTHORIZED)
	{
		if(param.contains(PARAM_PASSWORD_STR))
		{
			std::string password = param[PARAM_PASSWORD_STR].toString();
			m_password = password;
			pParam->remove(PARAM_PASSWORD_STR);
			Timestamp t;
			m_keepalive = t;
			return true;
		}
		else
		{
			detail = "Param error";
			return false;
		}
	}
	else
	{
		detail = "Unauthorized";
		return false;
	}
}

bool CUserClient::getPasswd(std::string& passwd)
{
	passwd = m_password;
	return true;
}

bool CUserClient::authorized()
{
	if(m_state == AUTHORIZED)
	{
		Timestamp t;
		m_keepalive = t;
		return true;
	}
	else
	{
		return false;
	}
}

bool CUserClient::logout(JSON::Object::Ptr param)
{
	m_challenge = "";
	m_username = "";
	m_password = "";
	m_token = "";
	m_state = UNAUTHORIZED;
	return true;
}


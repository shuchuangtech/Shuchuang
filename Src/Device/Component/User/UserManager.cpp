#include "Device/Component/User/UserManager.h"
#include "Poco/UUIDGenerator.h"
#include "Common/RPCDef.h"
#include "Common/PrintLog.h"
#include "Poco/FileStream.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/MD5Engine.h"
#include "Poco/Timespan.h"
using namespace Poco;
CUserManager::CUserManager()
{
	m_user_record = NULL;
	m_challenge_map.clear();
}

CUserManager::~CUserManager()
{
	m_user_record = NULL;
}

bool CUserManager::init(const std::string& dbPath)
{
	if(m_user_record != NULL)
		m_user_record = NULL;
	m_user_record = CUserRecord::instance();
	return m_user_record->init(dbPath);
}

bool CUserManager::verifyUser(std::string token)
{
	UserRecordNode userNode = {"", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	if(m_user_record->getUserByToken(userNode) == 0)
	{
		return false;
	}
	else
	{
		return checkUserValidity(userNode);
	}
	return false;
}

bool CUserManager::generateNewMD5String(std::string& md5str)
{
	UUIDGenerator uuid_gen;
	UUID uuid = uuid_gen.create();
	MD5Engine md5;
	md5.update(uuid.toString());
	const DigestEngine::Digest& digest = md5.digest();
	md5str = DigestEngine::digestToHex(digest);
	return true;
}

bool CUserManager::checkUserValidity(UserRecordNode& user)
{
	Timestamp timeOfValidity(user.timeOfValidity);
	DateTime dateTimeValidity(timeOfValidity);
	DateTime now;
	Timespan diff = dateTimeValidity - now;
	//if token has validity, use now and lastVerify to compute
	//here I don't care token's validity
	//timeOfValidity - now < 0 represents this user is out of validity
	if(diff.totalSeconds() < 0)
		return false;
	else
		return true;
}

bool CUserManager::verifyUserPassword(const std::string& username, const std::string& password, const std::string& prefix,  const std::string& challenge)
{
	UserRecordNode user_node = {"", "", 0, 0, 0, "", 0, 0};
	user_node.username = username;
	m_user_record->getUserByName(user_node);
	std::string prefix_password = prefix + user_node.password;
	MD5Engine md5;
	md5.update(prefix_password);
	const DigestEngine::Digest& prefix_digest = md5.digest();
	std::string prefix_md5(DigestEngine::digestToHex(prefix_digest));
	std::string challenge_password = prefix_md5 + challenge;
	md5.reset();
	md5.update(challenge_password);
	const DigestEngine::Digest& challenge_pass_digest = md5.digest();
	std::string md5_challenge_pass(DigestEngine::digestToHex(challenge_pass_digest));
	if(md5_challenge_pass == password)
		return true;
	else
		return false;
}

bool CUserManager::login(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_USERNAME_STR))
	{
		detail = "410";
		return false;
	}
	std::string username = pParam->getValue<std::string>(USER_USERNAME_STR);
	infof("%s, %d: User %s require to login.", __FILE__, __LINE__, username.c_str());
	UserRecordNode userNode = {"", "", 0, 0, 0, "", 0, 0};
	userNode.username = username;
	if(m_user_record->getUserByName(userNode) == 0)
	{
		warnf("%s, %d: User %s not exists.", __FILE__, __LINE__, username.c_str());
		detail = "411";
		return false;
	}
	m_map_mutex.lock();
	std::map<std::string, std::string>::iterator it = m_challenge_map.find(username);
	if(it == m_challenge_map.end())
		//login step 1. generate a new challenge and token
	{
		if(!checkUserValidity(userNode))
		{
			warnf("%s, %d: User %s is out of validity.", __FILE__, __LINE__, username.c_str());
			detail = "412";
			return false;
		}
		std::string challenge = "";
		generateNewMD5String(challenge);
		std::string token = "";
		generateNewMD5String(token);
		userNode.token = token;
		m_user_record->updateUser(userNode);
		m_challenge_map.insert(std::make_pair<std::string, std::string>(username, challenge));
		m_map_mutex.unlock();
		pParam->set(USER_CHALLENGE_STR, challenge);
		pParam->set(USER_TOKEN_STR, token);
		infof("%s, %d: User %s login step 1 finished, challenge:%s, token:%s.", __FILE__, __LINE__, username.c_str(), challenge.c_str(), token.c_str());
		return true;
	}
	else
		//login step 2. verify challenged password
	{
		std::string challenge = it->second;
		m_challenge_map.erase(it);
		m_map_mutex.unlock();
		if(!pParam->has(USER_TOKEN_STR))
		{
			detail = "413";
			return false;
		}
		std::string token = pParam->getValue<std::string>(USER_TOKEN_STR);
		UserRecordNode userNode = {"", "", 0, 0, 0, "", 0, 0};
		userNode.token = token;
		m_user_record->getUserByToken(userNode);
		if(!pParam->has(USER_PASSWORD_STR))
		{
			warnf("%s, %d: User %s login request param incomplete.", __FILE__, __LINE__, username.c_str());
			detail = "415";
			return false;
		}
		std::string password = pParam->getValue<std::string>(USER_PASSWORD_STR);
		pParam->remove(USER_PASSWORD_STR);
		if(verifyUserPassword(username, password, USER_METHOD_LOGIN, challenge))
		{
			Timestamp now;
			userNode.lastVerify = now.epochMicroseconds();
			m_user_record->updateUser(userNode);
			infof("%s, %d: User %s login step 2 finished, login successfully.", __FILE__, __LINE__, username.c_str());
			return true;
		}
		else
		{
			userNode.token = "";
			m_user_record->updateUser(userNode);
			warnf("%s, %d: User %s login step 2 password verify failed.", __FILE__, __LINE__, username.c_str());
			detail = "416";
			return false;
		}
	}
	warnf("%s, %d: User %s login unknown falure.", __FILE__, __LINE__, username.c_str());
	detail = "419";
	return false;
}

bool CUserManager::passwd(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_TOKEN_STR))
	{
		detail = "413";
		return false;
	}
	std::string token = pParam->getValue<std::string>(USER_TOKEN_STR);
	UserRecordNode userNode = {"", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	infof("%s, %d: User change password.", __FILE__, __LINE__);
	if(m_user_record->getUserByToken(userNode) == 0)
	{
		warnf("%s, %d: Can't find user by token %s.", __FILE__, __LINE__, token.c_str());
		detail = "414";
		return false;
	}
	if(!checkUserValidity(userNode))
	{
		warnf("%s, %d: User %s is out of validity.", __FILE__, __LINE__, userNode.username.c_str());
		detail = "412";
		return false;
	}
	std::string username = userNode.username;

	m_map_mutex.lock();
	std::map<std::string, std::string>::iterator it = m_challenge_map.find(username);
	if(it == m_challenge_map.end())
		//step 1
	{
		std::string challenge = "";
		generateNewMD5String(challenge);
		pParam->set(USER_CHALLENGE_STR, challenge);
		m_challenge_map.insert(std::make_pair<std::string, std::string>(username, challenge));
		m_map_mutex.unlock();
		infof("%s, %d: User %s change password step 1 finished, challenge:%s", __FILE__, __LINE__, username.c_str(), challenge.c_str());
		return true;
	}
	else
		//step 2
	{
		std::string challenge = it->second;
		m_challenge_map.erase(it);
		m_map_mutex.unlock();
		if(!pParam->has(USER_NEW_PASS_STR))
		{
			warnf("%s, %d", __FILE__, __LINE__);
			detail = "417";
			return false;
		}
		if(!pParam->has(USER_PASSWORD_STR))
		{
			warnf("%s, %d", __FILE__, __LINE__);
			detail = "413";
			return false;
		}
		std::string password = pParam->getValue<std::string>(USER_PASSWORD_STR);
		std::string newpass = pParam->getValue<std::string>(USER_NEW_PASS_STR);
		pParam->remove(USER_PASSWORD_STR);
		pParam->remove(USER_NEW_PASS_STR);
		if(verifyUserPassword(username, password, USER_METHOD_PASSWD, challenge))
		{
			userNode.password = newpass;
			//when change password successfully, need to login again
			userNode.token = "";
			m_user_record->updateUser(userNode);
			infof("%s, %d: User %s change password successfully, need to login again.", __FILE__, __LINE__, username.c_str());
			return true;
		}
		else
		{
			detail = "416";
			warnf("%s, %d: User %s change password verify failed.", __FILE__, __LINE__, username.c_str());
			return false;
		}
	}
	detail = "419";
	return false;
}

bool CUserManager::logout(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_TOKEN_STR))
	{
		detail = "413";
		return false;
	}
	std::string token = pParam->getValue<std::string>(USER_TOKEN_STR);
	UserRecordNode userNode = {"", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	infof("%s, %d: User logout.", __FILE__, __LINE__);
	if(m_user_record->getUserByToken(userNode) == 0)
	{
		warnf("%s, %d: Can't find user by token %s.", __FILE__, __LINE__, token.c_str());
		detail = "414";
		return false;
	}
	userNode.token = "";
	m_user_record->updateUser(userNode);
	infof("%s, %d; User %s logout successfully.", __FILE__, __LINE__, userNode.username.c_str());
	return true;
}


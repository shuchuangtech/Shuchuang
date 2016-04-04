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
	m_cache_map.clear();
	m_started = false;
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

bool CUserManager::start()
{
	if(m_started)
	{
		return true;
	}
	m_started = true;
	TimerCallback<CUserManager> callback(*this, &CUserManager::timerCallback);
	m_timer.setPeriodicInterval(60 * 1000);
	m_timer.start(callback);
	infof("%s, %d: CUserManager timer start successully.", __FILE__, __LINE__);
	return true;
}

bool CUserManager::stop()
{
	if(!m_started)
	{
		return true;
	}
	m_started = false;
	m_timer.stop();
	infof("%s, %d: CUserManager timer stop successfully.", __FILE__, __LINE__);
	return true;
}

bool CUserManager::userOpenDoor(const std::string& token)
{
	m_cache_mutex.lock();
	std::map<std::string, int>::iterator it = m_cache_map.find(token);
	if(it == m_cache_map.end())
	{
		m_cache_map.insert(std::make_pair<std::string, int>(token, 1));
	}
	else
	{
		it->second = it->second + 1;
	}
	m_cache_mutex.unlock();
	return true;
}

void CUserManager::timerCallback(Timer& timer)
{
	m_cache_mutex.lock();
	std::map<std::string, int>::iterator it;
	for(it = m_cache_map.begin(); it != m_cache_map.end(); )
	{
		std::map<std::string, int>::iterator itemp = it++;
		std::string token = itemp->first;
		UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
		userNode.token = token;
		m_user_record->getUserByToken(userNode);
		if(userNode.remainOpen > 0)
		{
			userNode.remainOpen = userNode.remainOpen - itemp->second;
			m_user_record->updateUser(userNode);
			infof("%s, %d: User %s remainOpen minus %d.", __FILE__, __LINE__, userNode.username.c_str(), userNode.remainOpen);
		}
		m_cache_map.erase(itemp);
	}
	m_cache_mutex.unlock();
	m_challenge_mutex.lock();
	std::map<std::string, ChallengeNodePtr>::iterator it2;
	for(it2 = m_challenge_map.begin(); it2 != m_challenge_map.end(); )
	{
		std::map<std::string, ChallengeNodePtr>::iterator itemp = it2++;
		DateTime now;
		Timespan diff = now - itemp->second->generateTime;
		if(diff.totalSeconds() > 60)
		{
			itemp->second = NULL;
			m_challenge_map.erase(itemp);
		}
	}
	m_challenge_mutex.unlock();
}

bool CUserManager::canUserOpenDoor(const std::string& token)
{
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	m_user_record->getUserByToken(userNode);
	if(userNode.remainOpen < 0)
		return true;
	m_cache_mutex.lock();
	std::map<std::string, int>::iterator it = m_cache_map.find(token);
	m_cache_mutex.unlock();
	if(it == m_cache_map.end())
	{
		if(userNode.remainOpen > 0)
			return true;
		else
			return false;
	}
	else
	{
		if(userNode.remainOpen - it->second > 0)
			return true;
		else
			return false;
	}
}

bool CUserManager::getUserNameFromToken(const std::string& token, std::string& username)
{
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	if(m_user_record->getUserByToken(userNode) != 0)
	{
		username = userNode.username;
		return true;
	}
	else
	{
		return false;
	}

}

int CUserManager::userAuthority(const std::string& token)
{
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	if(m_user_record->getUserByToken(userNode) != 0)
	{
		return userNode.authority;
	}
	else
	{
		return -1;
	}
}

bool CUserManager::verifyUser(std::string token)
{
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
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
	UserRecordNode user_node = {"", "", "", 0, 0, 0, "", 0, 0};
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
	if(pParam.isNull() || !pParam->has(USER_USERNAME_STR) || !pParam->has(USER_BINDUSER_STR))
	{
		detail = "400";
		return false;
	}
	std::string username = pParam->getValue<std::string>(USER_USERNAME_STR);
	infof("%s, %d: User %s require to login.", __FILE__, __LINE__, username.c_str());
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.username = username;
	if(m_user_record->getUserByName(userNode) == 0)
	{
		warnf("%s, %d: User %s not exists.", __FILE__, __LINE__, username.c_str());
		detail = "411";
		return false;
	}
	m_challenge_mutex.lock();
	std::map<std::string, ChallengeNodePtr>::iterator it = m_challenge_map.find(username);
	if(it == m_challenge_map.end())
		//login step 1. generate a new challenge and token
	{
		if(!checkUserValidity(userNode))
		{
			warnf("%s, %d: User %s is out of validity.", __FILE__, __LINE__, username.c_str());
			detail = "402";
			m_challenge_mutex.unlock();
			return false;
		}
		std::string binduser = pParam->getValue<std::string>(USER_BINDUSER_STR);
		if(username == "admin")
		{
			if(!userNode.binduser.empty() && userNode.binduser != binduser)
			{
				warnf("%s, %d: User %s is binded by %s.", __FILE__, __LINE__, username.c_str());
				detail = "418";
				pParam->remove(USER_BINDUSER_STR);
				pParam->set(USER_BINDUSER_STR, userNode.binduser);
				m_challenge_mutex.unlock();
				return false;
			}
		}
		else
		{
			if(userNode.binduser != binduser)
			{
				warnf("%s, %d: User %s is binded by %s.", __FILE__, __LINE__, username.c_str());
				detail = "418";
				pParam->remove(USER_BINDUSER_STR);
				pParam->set(USER_BINDUSER_STR, userNode.binduser);
				m_challenge_mutex.unlock();
				return false;
			}
		}
		std::string challenge = "";
		generateNewMD5String(challenge);
		std::string token = "";
		generateNewMD5String(token);
		userNode.token = token;
		userNode.binduser = pParam->getValue<std::string>(USER_BINDUSER_STR);
		m_user_record->updateUser(userNode);
		ChallengeNodePtr pChallenge = new ChallengeNode;
		pChallenge->challenge = challenge;
		m_challenge_map.insert(std::make_pair<std::string, ChallengeNodePtr>(username, pChallenge));
		m_challenge_mutex.unlock();
		pParam->set(USER_CHALLENGE_STR, challenge);
		pParam->set(USER_TOKEN_STR, token);
		infof("%s, %d: User %s login binded by %s step 1 finished, challenge:%s, token:%s.", __FILE__, __LINE__, username.c_str(), userNode.binduser.c_str(),challenge.c_str(), token.c_str());
		return true;
	}
	else
		//login step 2. verify challenged password
	{
		std::string challenge = it->second->challenge;
		m_challenge_map.erase(it);
		m_challenge_mutex.unlock();
		if(!pParam->has(USER_TOKEN_STR))
		{
			detail = "400";
			return false;
		}
		std::string token = pParam->getValue<std::string>(USER_TOKEN_STR);
		UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
		userNode.token = token;
		m_user_record->getUserByToken(userNode);
		if(!pParam->has(USER_PASSWORD_STR))
		{
			warnf("%s, %d: User %s login request param incomplete.", __FILE__, __LINE__, username.c_str());
			detail = "400";
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
		detail = "400";
		return false;
	}
	std::string token = pParam->getValue<std::string>(USER_TOKEN_STR);
	if(!verifyUser(token))
	{
		detail = "402";
		return false;
	}
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	infof("%s, %d: User change password.", __FILE__, __LINE__);
	std::string username = userNode.username;

	m_challenge_mutex.lock();
	std::map<std::string, ChallengeNodePtr>::iterator it = m_challenge_map.find(username);
	if(it == m_challenge_map.end())
		//step 1
	{
		std::string challenge = "";
		generateNewMD5String(challenge);
		ChallengeNodePtr pChallenge = new ChallengeNode;
		pChallenge->challenge = challenge;
		pParam->set(USER_CHALLENGE_STR, challenge);
		m_challenge_map.insert(std::make_pair<std::string, ChallengeNodePtr>(username, pChallenge));
		m_challenge_mutex.unlock();
		infof("%s, %d: User %s change password step 1 finished, challenge:%s", __FILE__, __LINE__, username.c_str(), challenge.c_str());
		return true;
	}
	else
		//step 2
	{
		std::string challenge = it->second->challenge;
		m_challenge_map.erase(it);
		m_challenge_mutex.unlock();
		if(!pParam->has(USER_NEW_PASS_STR) || !pParam->has(USER_PASSWORD_STR))
		{
			warnf("%s, %d", __FILE__, __LINE__);
			detail = "400";
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

bool CUserManager::addUser(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_USERNAME_STR) || !pParam->has(USER_BINDUSER_STR) || !pParam->has(USER_PASSWORD_STR) || !pParam->has(USER_AUTHORITY_STR) || !pParam->has(USER_REMAINOPEN_STR) || !pParam->has(USER_TIMEOFVALIDITY_STR) || !pParam->has(REG_TOKEN_STR))
	{
		detail = "400";
		return false;
	}
	std::string token = pParam->getValue<std::string>(REG_TOKEN_STR);
	std::string password = pParam->getValue<std::string>(USER_PASSWORD_STR);
	pParam->remove(REG_TOKEN_STR);
	pParam->remove(USER_PASSWORD_STR);
	if(!verifyUser(token))
	{
		detail = "402";
		return false;
	}
	UserRecordNode loginUser = {"", "", "", 0, 0, 0, "", 0, 0};
	loginUser.token = token;
	m_user_record->getUserByToken(loginUser);
	std::string username = pParam->getValue<std::string>(USER_USERNAME_STR);
	std::string binduser = pParam->getValue<std::string>(USER_BINDUSER_STR);
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.username = username;
	if(m_user_record->getUserByName(userNode) != 0)
	{
		warnf("%s, %d: User %s already exists.", __FILE__, __LINE__, username.c_str());
		detail = "410";
		return false;
	}
	userNode.password = password;
	userNode.binduser = pParam->getValue<std::string>(USER_BINDUSER_STR);
	int authority = pParam->getValue<int>(USER_AUTHORITY_STR);
	if(loginUser.authority != USER_AUTHORITY_ADMIN || authority < USER_AUTHORITY_TEMP || authority > USER_AUTHORITY_LONG)
	{
		warnf("%s, %d: Add user authority invalid.", __FILE__, __LINE__);
		detail = "412";
		return false;
	}
	userNode.authority = pParam->getValue<int>(USER_AUTHORITY_STR);
	userNode.remainOpen = pParam->getValue<int>(USER_REMAINOPEN_STR);
	userNode.timeOfValidity = pParam->getValue<Int64>(USER_TIMEOFVALIDITY_STR);
	if(!m_user_record->addUser(userNode))
	{
		warnf("%s, %d: User %s add failed.", __FILE__, __LINE__);
		detail = "413";
		return false;
	}
	infof("%s, %d: User[username:%s, authority:%d, timeOfValidity:%ld, remainOpen:%d, bindUser:%s] add successully.", __FILE__, __LINE__, userNode.username.c_str(), userNode.authority, userNode.timeOfValidity, userNode.remainOpen, userNode.binduser.c_str());
	return true;
}

bool CUserManager::deleteUser(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_USERNAME_STR) || !pParam->has(REG_TOKEN_STR))
	{
		detail = "400";
		return false;
	}
	std::string token = pParam->getValue<std::string>(REG_TOKEN_STR);
	pParam->remove(REG_TOKEN_STR);
	if(!verifyUser(token))
	{
		detail = "402";
		return false;
	}
	UserRecordNode deleteUser = {"", "", "", 0, 0, 0, "", 0, 0};
	deleteUser.username = pParam->getValue<std::string>(USER_USERNAME_STR);
	if(m_user_record->getUserByName(deleteUser) == 0)
	{
		detail = "411";
		return false;
	}
	UserRecordNode loginUser = {"", "", "", 0, 0, 0, "", 0, 0};
	loginUser.token = token;
	m_user_record->getUserByToken(loginUser);
	if(loginUser.authority != USER_AUTHORITY_ADMIN || deleteUser.authority > USER_AUTHORITY_LONG)
	{
		warnf("%s, %d: User %s has no authority to delete users.", __FILE__, __LINE__, loginUser.username.c_str());
		detail = "412";
		return false;
	}
	if(!m_user_record->deleteUser(deleteUser))
	{
		detail = "414";
		return false;
	}
	infof("%s, %d: User %s deleted.", __FILE__, __LINE__, deleteUser.username.c_str());
	return true;
}

bool CUserManager::topUpUser(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_USERNAME_STR) || !pParam->has(REG_TOKEN_STR) || !pParam->has(USER_TIMEOFVALIDITY_STR) || !pParam->has(USER_REMAINOPEN_STR))
	{
		detail = "400";
		return false;
	}
	std::string token = pParam->getValue<std::string>(REG_TOKEN_STR);
	pParam->remove(REG_TOKEN_STR);
	if(!verifyUser(token))
	{
		detail = "402";
		return false;
	}
	UserRecordNode loginNode = {"", "", "", 0, 0, 0, "", 0, 0};
	loginNode.token = token;
	m_user_record->getUserByToken(loginNode);
	if(loginNode.authority != USER_AUTHORITY_ADMIN)
	{
		detail = "412";
		return false;
	}
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.username = pParam->getValue<std::string>(USER_USERNAME_STR);
	if(m_user_record->getUserByName(userNode) == 0)
	{
		detail = "410";
		return false;
	}
	Int64 timeOfValidity = pParam->getValue<Int64>(USER_TIMEOFVALIDITY_STR);
	int remainOpen = pParam->getValue<int>(USER_REMAINOPEN_STR);
	Timestamp now;
	if(timeOfValidity > now.epochMicroseconds() && timeOfValidity > userNode.timeOfValidity && timeOfValidity < loginNode.timeOfValidity)
	{
		userNode.timeOfValidity = timeOfValidity;
	}
	userNode.remainOpen = remainOpen;
	if(!m_user_record->updateUser(userNode))
	{
		warnf("%s, %d: TopUp User timeOfValidity or remainOpen failed.", __FILE__, __LINE__);
		detail = "417";
		return false;
	}
	infof("%s, %d: User[%s] top up successfully, timeOfValidity:%ld, remainOpen:%d.", __FILE__, __LINE__, userNode.username.c_str(), timeOfValidity, remainOpen);
	return true;
}

bool CUserManager::logout(JSON::Object::Ptr& pParam, std::string& detail)
{
	if(pParam.isNull() || !pParam->has(USER_TOKEN_STR))
	{
		detail = "400";
		return false;
	}
	std::string token = pParam->getValue<std::string>(USER_TOKEN_STR);
	UserRecordNode userNode = {"", "", "", 0, 0, 0, "", 0, 0};
	userNode.token = token;
	infof("%s, %d: User logout.", __FILE__, __LINE__);
	if(m_user_record->getUserByToken(userNode) == 0)
	{
		warnf("%s, %d: Can't find user by token %s.", __FILE__, __LINE__, token.c_str());
		detail = "402";
		return false;
	}
	userNode.token = "";
	userNode.binduser = "";
	m_user_record->updateUser(userNode);
	infof("%s, %d; User %s logout and unbind successfully.", __FILE__, __LINE__, userNode.username.c_str());
	return true;
}

bool CUserManager::listUser(JSON::Object::Ptr& pParam, std::string& detail)
{
	std::string token = pParam->getValue<std::string>(REG_TOKEN_STR);
	pParam->remove(REG_TOKEN_STR);
	if(userAuthority(token) != USER_AUTHORITY_ADMIN)
	{
		warnf("%s, %d: User has no authority to list users.", __FILE__, __LINE__);
		detail = "412";
		return false;
	}
	if(pParam.isNull() || !pParam->has(USER_LIMIT_STR) || !pParam->has(USER_OFFSET_STR))
	{
		detail = "400";
		return false;
	}
	int limit = pParam->getValue<int>(USER_LIMIT_STR);
	int offset = pParam->getValue<int>(USER_OFFSET_STR);
	std::vector<UserRecordNode> data_set;
	data_set.clear();
	int ret = m_user_record->getUsers(limit, offset, data_set);
	if(ret < 0)
	{
		warnf("%s, %d: Get user record failed.", __FILE__, __LINE__);
		detail = "415";
		return false;
	}
	JSON::Array::Ptr pArray = new JSON::Array;;
	for(int i = 0; i < ret; i++)
	{
		DynamicStruct ds;
		ds["id"] = i;
		ds[USER_BINDUSER_STR] = data_set[i].binduser;
		ds[USER_TIMEOFVALIDITY_STR] = data_set[i].timeOfValidity;
		ds[USER_REMAINOPEN_STR] = data_set[i].remainOpen;
		ds[USER_USERNAME_STR] = data_set[i].username;
		ds[USER_AUTHORITY_STR] = data_set[i].authority;
		pArray->add(ds);
	}
	pParam->set("Users", pArray);
	return true;
}


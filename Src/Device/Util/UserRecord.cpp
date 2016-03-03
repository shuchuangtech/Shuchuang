#include "Device/Util/UserRecord.h"
#include "Common/PrintLog.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/RecordSet.h"
#include <fstream>
using namespace Poco;
using namespace Poco::Data;
using namespace Poco::Data::Keywords;
CUserRecord::CUserRecord()
{
	m_session_ptr = NULL;
}

CUserRecord::~CUserRecord()
{
	if(m_session_ptr != NULL)
	{
		m_session_ptr->close();
		delete m_session_ptr;
		m_session_ptr = NULL;
	}
}

bool CUserRecord::init(const std::string& dbPath)
{
	SQLite::Connector::registerConnector();
	try
	{
		m_session_ptr = new Session("SQLite",dbPath);
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Init UserRecord with %s failed.", __FILE__, __LINE__, dbPath.c_str());
		return false;
	}
	infof("%s, %d: Init UserRecord with %s successfully.", __FILE__, __LINE__, dbPath.c_str());
	/*
	try
	{
		Statement create(*m_session_ptr);
		create << "CREATE TABLE IF NOT EXISTS `User` ("
			<< "`Id` INTEGER PRIMARY KEY AUTOINCREMENT,"
			<< "`Username` VARCHAR(64) NOT NULL UNIQUE,"
			<< "`Password` VARCHAR(64) NOT NULL,"
			<< "`Authority` TINYINT,"
			<< "`TimeOfValidity` BIGINT,"
			<< "`RemainOpen` INTEGER,"
			<< "`Token` VARCHAR(64),"
			<< "`LastVerify` BIGINT,"
			<< "`LastLogin` BIGINT)";
		create.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Create table User failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	*/
	return true;
}

bool CUserRecord::resetUser(const std::string& dbPath, const std::string& bakDbPath)
{
	if(m_session_ptr != NULL)
	{	
		m_session_ptr->close();
		delete m_session_ptr;
	}
	std::ifstream ifs(bakDbPath.c_str(), std::ios::in|std::ios::binary);
	std::ofstream ofs(dbPath.c_str(), std::ios::out|std::ios::trunc|std::ios::binary);
	ofs << ifs.rdbuf();
	ifs.close();
	ofs.close();
	try
	{
		m_session_ptr = new Session("SQLite",dbPath);
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Open UserRecord with %s failed.", __FILE__, __LINE__, dbPath.c_str());
		return false;
	}
	infof("%s, %d: UserRecord reset successfully.", __FILE__, __LINE__);
	return true;
}

int CUserRecord::addUser(UserRecordNode& user)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	if(user.username.empty() 
		|| user.password.empty())
	{
		warnf("%s, %d: Username or Password is NULL.", __FILE__, __LINE__);
		return -1;
	}
	Statement sinsert(*m_session_ptr);
	sinsert << "INSERT INTO `User` (`Username`, `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin`)"
	<< "VALUES(?, ?, ?, ?, ?, ?, ?, ?)",
				use(user.username),
				use(user.password),
				use(user.authority),
				use(user.timeOfValidity),
				use(user.remainOpen),
				use(user.token),
				use(user.lastVerify),
				use(user.lastLogin);
	int ret = 0;
	try
	{
		ret = sinsert.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Insert into User failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return 0;
	}
	return ret;
}

int CUserRecord::deleteUser(UserRecordNode& user)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	if(user.username.empty())
	{
		warnf("%s, %d: Username is NULL." __FILE__, __LINE__);
		return -1;
	}
	Statement sdelete(*m_session_ptr);
	sdelete << "DELETE FROM `User` WHERE `Username`=?;",
		use(user.username);
	int ret = 0;
	try
	{
		ret = sdelete.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Delete from table failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return 0;
	}
	return ret;
}

int CUserRecord::getUserByName(UserRecordNode& user)
{
	return getSingleUser("Username", user);
}

int CUserRecord::getUserByToken(UserRecordNode& user)
{
	return getSingleUser("Token", user);
}

int CUserRecord::getUsersByAuth(int auth, std::vector<UserRecordNode>(& data_set))
{
	return getMultiUsers("Authority", auth, data_set);
}

int CUserRecord::getUsersByOpen(int open, std::vector<UserRecordNode>(& data_set))
{
	return getMultiUsers("RemainOpen", open, data_set);
}

int CUserRecord::getAllUsers(std::vector<UserRecordNode>(& data_set))
{
	return getMultiUsers("All", 0, data_set);
}

int CUserRecord::getSingleUser(const std::string& col_name, UserRecordNode& user)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	
	Statement sselect(*m_session_ptr);
	if(col_name == "Username")
	{
		sselect << "SELECT `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin` FROM `User` WHERE `Username`=?",
			into(user.password),
			into(user.authority),
			into(user.timeOfValidity),
			into(user.remainOpen),
			into(user.token),
			into(user.lastVerify),
			into(user.lastLogin),
			use(user.username);
	}
	else if(col_name == "Token")
	{
		sselect << "SELECT `Username`, `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `LastVerify`, `LastLogin` FROM `User` WHERE `Token`=?",
			into(user.username),
			into(user.password),
			into(user.authority),
			into(user.timeOfValidity),
			into(user.remainOpen),
			into(user.lastVerify),
			into(user.lastLogin),
			use(user.token);
	}
	else
	{
		warnf("%s, %d: Not supported select col name[%s].", __FILE__, __LINE__, col_name.c_str());
		return -1;
	}
	int ret = 0;
	try
	{
		ret = sselect.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Select single from User failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return 0;
	}
	return ret;
}

int CUserRecord::getMultiUsers(const std::string& col_name, int col_value, std::vector<UserRecordNode>(& data_set))
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	Statement sselect(*m_session_ptr);
	if(col_name == "Authority")
	{	
		sselect << "SELECT `Username`, `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin` FROM `User` WHERE `Authority`=?",
				use(col_value);
	}
	else if(col_name == "RemainOpen")
	{
		sselect << "SELECT `Username`, `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin` FROM `User` WHERE `RemainOpen`=?",
				use(col_value);
	}
	else if(col_name == "All")
	{
		sselect << "SELECT `Username`, `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin` FROM `User`";
	}
	else
	{
		warnf("%s, %d: Not supported select col name[%s].", __FILE__, __LINE__, col_name.c_str());
		return -1;
	}
	int ret = 0;
	try
	{
		ret = sselect.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Select multi from User failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return -1;
	}
	RecordSet rs(sselect);
	bool more = rs.moveFirst();
	while(more)
	{
		UserRecordNode user_node;
		std::size_t col = 0;
		user_node.username = rs[col++].convert<std::string>();
		user_node.password = rs[col++].convert<std::string>();
		user_node.authority = rs[col++].convert<int>();
		user_node.timeOfValidity = rs[col++].convert<Int64>();
		user_node.remainOpen = rs[col++].convert<int>();
		user_node.token = rs[col++].convert<std::string>();
		user_node.lastVerify = rs[col++].convert<Int64>();
		user_node.lastLogin = rs[col++].convert<Int64>();
		more = rs.moveNext();
		data_set.push_back(user_node);
	}
	return ret;
}

int CUserRecord::updateUser(UserRecordNode& user)
{

	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	if(user.username.empty() || user.password.empty())
	{
		warnf("%s, %d: Username or password is NULL.", __FILE__, __LINE__);
		return -1;
	}
	Statement supdate(*m_session_ptr);
	supdate << "UPDATE `User` SET `Password`=?, `Authority`=?, `TimeOfValidity`=?, `RemainOpen`=?, `Token`=?, `LastVerify`=?, `LastLogin`=? WHERE `Username`=?",
		use(user.password),
		use(user.authority),
		use(user.timeOfValidity),
		use(user.remainOpen),
		use(user.token),
		use(user.lastVerify),
		use(user.lastLogin),
		use(user.username);
	int ret = 0;
	try
	{
		ret = supdate.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Update User failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return 0;
	}
	return ret;
}


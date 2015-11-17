#include "Device/Util/UserRecord.h"
#include "Common/PrintLog.h"
#include "Poco/Data/SQLite/Connector.h"
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
	m_session_ptr = new Session("SQLite",dbPath);
	try
	{
		Statement create(*m_session_ptr);
		create << "CREATE TABLE IF NOT EXISTS `User` ("
			<< "`Id` INTEGER PRIMARY KEY AUTOINCREMENT,"
			<< "`Username` VARCHAR(30) NOT NULL UNIQUE,"
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

int CUserRecord::getUser(UserRecordNode& user)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	if(user.username.empty())
	{
		warnf("%s, %d: Username is NULL.", __FILE__, __LINE__);
		return -1;
	}
	Statement sselect(*m_session_ptr);
	sselect << "SELECT `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin` FROM `User` WHERE `Username`=?",
		into(user.password),
		into(user.authority),
		into(user.timeOfValidity),
		into(user.remainOpen),
		into(user.token),
		into(user.lastVerify),
		into(user.lastLogin),
		use(user.username);
	int ret = 0;
	try
	{
		ret = sselect.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Select from User failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return 0;
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


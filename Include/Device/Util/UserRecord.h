#ifndef __DEVICE_UTIL_USER_RECORD_H__
#define __DEVICE_UTIL_USER_RECORD_H__
#include "Poco/SingletonHolder.h"
#include "Poco/Types.h"
#include "Poco/Data/Session.h"
#include <vector>
//data structure
struct _UserRecordNode
{
	enum AuthLevel
	{
		LEVEL_ADMIN = 9,
		LEVEL_LONGTERM = 8,
		LEVEL_TEMP = 7
	};
	//Timestamp in UTC
	std::string		username;
	std::string		password;
	std::string		binduser;
	int				authority;
	Poco::Int64		timeOfValidity;
	int				remainOpen;
	std::string		token;
	Poco::Int64		lastVerify;
	Poco::Int64		lastLogin;
};
typedef struct _UserRecordNode UserRecordNode;

//class structure
class CUserRecord
{
public:
	CUserRecord();
	~CUserRecord();
	static CUserRecord* instance()
	{
		static Poco::SingletonHolder<CUserRecord> sh;
		return sh.get();
	}
	bool init(const std::string& dbPath);
	bool resetUser(const std::string& dbPath, const std::string& bakDbPath);
	int addUser(UserRecordNode&);
	int deleteUser(UserRecordNode&);
	int updateUser(UserRecordNode&);
	int getUserByName(UserRecordNode&);
	int getUserByToken(UserRecordNode&);
	int getUsers(int limit, int offset, std::vector<UserRecordNode>&);
private:
	int getSingleUser(const std::string&, UserRecordNode&);
	Poco::Data::Session*		m_session_ptr;
};
#endif


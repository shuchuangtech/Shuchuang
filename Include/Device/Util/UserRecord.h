#ifndef __DEVICE_UTIL_USER_RECORD_H__
#define __DEVICE_UTIL_USER_RECORD_H__
#include "Poco/SingletonHolder.h"
#include "Poco/Types.h"
#include "Poco/Data/Session.h"
#include "Poco/SharedPtr.h"
//data structure
struct _UserRecordNode
{
	enum AuthLevel
	{
		LEVEL_ADMIN = 9,
		LEVEL_LONGTERM = 8,
		LEVEL_TEMP = 7
	};
	std::string		username;
	std::string		password;
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
	int addUser(UserRecordNode&);
	int deleteUser(UserRecordNode&);
	int updateUser(UserRecordNode&);
	int getUser(UserRecordNode&);
private:
	Poco::Data::Session*		m_session_ptr;
};
#endif


#ifndef __SERVER_APNS_APPLE_PUSH_H__
#define __SERVER_APNS_APPLE_PUSH_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
class CApplePush
{
public:
	static CApplePush* instance()
	{
		static Poco::SingletonHolder<CApplePush> sh;
		return sh.get();
	}
	CApplePush();
	~CApplePush();
	bool init();
	bool pushBmob(const std::string& device_token, const std::string& content);
private:
	Poco::JSON::Object::Ptr	m_bmob_info;
};
#endif


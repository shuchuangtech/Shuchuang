#ifndef __SERVER_PUSH_CENTER_H__
#define __SERVER_PUSH_CENTER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
class CPushCenter
{
public:
	static CPushCenter* instance()
	{
		static Poco::SingletonHolder<CPushCenter> sh;
		return sh.get();
	}
	CPushCenter();
	~CPushCenter();
	bool init();
	bool pushBmobIOS(const std::string& device_token, const std::string& content);
	bool pushBmobAndroid(const std::string& installation_id, const std::string& content);
private:
	Poco::JSON::Object::Ptr	m_bmob_info;
};

#endif


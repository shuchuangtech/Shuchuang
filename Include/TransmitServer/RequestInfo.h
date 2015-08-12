#ifndef __SERVER_REQUEST_INFO_H__
#define __SERVER_REQUEST_INFO_H__
#include "Poco/Types.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Object.h"
#include "Poco/Semaphore.h"
struct _RequestInfo
{
	_RequestInfo(const Poco::UInt64 src, const std::string& uid, Poco::UInt64 t, Poco::JSON::Object::Ptr obj)
		:src_id(src), uuid(uid), timeout(t), request(obj), response(NULL), sem(0, 1)
	{
	}
	Poco::UInt64 src_id;
	std::string uuid;
	Poco::UInt64 timeout;
	Poco::JSON::Object::Ptr request;
	Poco::JSON::Object::Ptr response;
	Poco::Semaphore	sem;
};
typedef struct _RequestInfo RequestInfo;
#endif


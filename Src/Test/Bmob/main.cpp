#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/MediaType.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/JSON/Array.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Timespan.h"
#include <iostream>
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	char url[64] = {0, };
	snprintf(url, 63, "api.bmob.cn/1/push");
	Context::Ptr pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE, 9, false, "EECDH+aRSA+AESGCM");
	printf("url %s\n", url);
	HTTPSClientSession https("api.bmob.cn", HTTPSClientSession::HTTPS_PORT, pContext);
	printf("1111111111\n");
	HTTPRequest request;
	request.setHost("api.bmob.cn");
	request.setURI("/1/push");
	request.setContentType(MediaType("application", "json"));
	request.setMethod("POST");
	request.add("X-Bmob-Application-Id", "27f1f3599a223cfa40bb5c5e5daedd7a");
	request.add("X-Bmob-REST-API-KEY", "0952518ed9dd101fab4c5c02d957f62d");
	printf("2222222\n");
	DynamicStruct ds;
	DynamicStruct where;
//	where["deviceToken"] = "ec77339fd0393bd84155b383501e368eb116801532ffcba775bf0d208f384d0a";
	where["deviceToken"] = "91cd38304c5ccad474710466f8a30e6bc2cdebd52e6e5d97082bb3d283ed1bd4";
	ds["where"] = where;
	DynamicStruct data;
	data["alert"] = "大王别撸了";
	data["badge"] = 1;
	data["sound"] = "default";
	data["mykey"] = "myvalue";
	ds["data"] = data;
	std::string content = ds.toString();
	printf("send:%s\n", content.c_str());
	request.setContentLength(content.length());
	printf("22223333\n");
	std::ostream& ostr = https.sendRequest(request);
	printf("3333\n");
	ostr << content << std::flush;
	printf("4444\n");
	HTTPResponse response;
	https.setTimeout(Timespan(20, 0));
	std::string recv;
	try
	{
		std::istream& istr = https.receiveResponse(response);
		char buf[512] = {0, };
		while(!istr.eof())
		{
			istr.read(buf, 512);
			recv += buf;
			memset(buf, 0, 512);
		}
	}
	catch(Exception& e)
	{
		printf("receive response error %s.\n", e.message().c_str());

	}
	printf("recv:%s.\n", recv.c_str());
	return 0;
}


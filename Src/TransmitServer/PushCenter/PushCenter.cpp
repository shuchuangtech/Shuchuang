#include "TransmitServer/PushCenter/PushCenter.h"
#include "Common/PrintLog.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/MediaType.h"
#include "Common/ConfigManager.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/Struct.h"
using namespace Poco;
using namespace Poco::Net;
CPushCenter::CPushCenter()
{
	m_bmob_info = NULL;
}

CPushCenter::~CPushCenter()
{
}

bool CPushCenter::init()
{
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pAPNS;
	config->getConfig("APNS", pAPNS);
	if(pAPNS.isNull())
	{
		warnf("%s, %d: Push center service init get config failed.", __FILE__, __LINE__);
		return false;
	}
	JSON::Array::Ptr pArray = pAPNS->getArray("Proxy");
	for(std::size_t i = 0; i < pArray->size(); i++)
	{
		std::string proxy = pArray->getElement<std::string>(i);
		if(proxy == "bmob")
		{
			m_bmob_info = pAPNS->getObject("bmob");
			if(m_bmob_info.isNull() || !m_bmob_info->has("Host") || !m_bmob_info->has("URI") || !m_bmob_info->has("APPID") || !m_bmob_info->has("APPKey"))
			{
				warnf("%s, %d: Push center service init read bmob config failed.", __FILE__, __LINE__);
				m_bmob_info = NULL;
				return false;
			}
		}
	}
	return true;
}

bool CPushCenter::pushBmobIOS(const std::string& device_token, const std::string& content)
{
	Context::Ptr pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE, 9, false, "EECDH+aRSA+AESGCM");
	std::string host = m_bmob_info->getValue<std::string>("Host");
	std::string uri = m_bmob_info->getValue<std::string>("URI");
	std::string appId = m_bmob_info->getValue<std::string>("APPID");
	std::string appKey = m_bmob_info->getValue<std::string>("APPKey");
	HTTPSClientSession https(host, HTTPSClientSession::HTTPS_PORT, pContext);
	HTTPRequest request;
	request.setHost(host);
	request.setURI(uri);
	request.setContentType(MediaType("application", "json"));
	request.setMethod("POST");
	request.add("X-Bmob-Application-Id", appId);
	request.add("X-Bmob-REST-API-KEY", appKey);
	DynamicStruct where;
	where["deviceToken"] = device_token;
	DynamicStruct data;
	data["alert"] = content;
	data["sound"] = "default";
	DynamicStruct ds;
	ds["where"] = where;
	ds["data"] = data;
	std::string dsStr = ds.toString();
	request.setContentLength(dsStr.length());
	std::ostream& ostr = https.sendRequest(request);
	ostr << dsStr << std::flush;
	infof("%s, %d: Push center service push notification[%s] through bmob.", __FILE__, __LINE__, dsStr.c_str());
	HTTPResponse response;
	https.setTimeout(Timespan(10, 0));
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
		warnf("%s, %d: Push center service bmob receive response error[%s].", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	infof("%s, %d: Push center service bmob receice:%s", __FILE__, __LINE__);
	return true;
}

bool CPushCenter::pushBmobAndroid(const std::string& installation_id, const std::string& content)
{
	Context::Ptr pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE, 9, false, "EECDH+aRSA+AESGCM");
	std::string host = m_bmob_info->getValue<std::string>("Host");
	std::string uri = m_bmob_info->getValue<std::string>("URI");
	std::string appId = m_bmob_info->getValue<std::string>("APPID");
	std::string appKey = m_bmob_info->getValue<std::string>("APPKey");
	HTTPSClientSession https(host, HTTPSClientSession::HTTPS_PORT, pContext);
	HTTPRequest request;
	request.setHost(host);
	request.setURI(uri);
	request.setContentType(MediaType("application", "json"));
	request.setMethod("POST");
	request.add("X-Bmob-Application-Id", appId);
	request.add("X-Bmob-REST-API-KEY", appKey);
	DynamicStruct where;
	where["installationId"] = installation_id;
	DynamicStruct data;
	data["alert"] = content;
	DynamicStruct ds;
	ds["where"] = where;
	ds["data"] = data;
	std::string dsStr = ds.toString();
	request.setContentLength(dsStr.length());
	std::ostream& ostr = https.sendRequest(request);
	ostr << dsStr << std::flush;
	infof("%s, %d: Android Push service push notification[%s] through bmob.", __FILE__, __LINE__, dsStr.c_str());
	HTTPResponse response;
	https.setTimeout(Timespan(10, 0));
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
		warnf("%s, %d: Push center service bmob receive response error[%s].", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	infof("%s, %d: Push center service bmob receice:%s", __FILE__, __LINE__);
	return true;

}


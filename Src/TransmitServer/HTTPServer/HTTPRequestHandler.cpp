#include "TransmitServer/HTTPServer/HTTPRequestHandler.h"
#include "TransmitServer/RegServer/RequestInfo.h"
#include "Common/RPCDef.h"
#include "Common/PrintLog.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Struct.h"
#include "TransmitServer/RegServer/RegServer.h"
#include "TransmitServer/DeviceManager.h"
using namespace Poco;
using namespace Poco::Net;
CHTTPRequestHandler::CHTTPRequestHandler()
{
	m_buf = NULL;
}

CHTTPRequestHandler::~CHTTPRequestHandler()
{
	if(m_buf != NULL)
		delete[] m_buf;
}

bool CHTTPRequestHandler::checkRequestFormat(JSON::Object::Ptr& request, JSON::Object::Ptr& response)
{
	DynamicStruct ds = *request;
	response->remove(KEY_TYPE_STR);
	response->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
	if(!ds.contains(KEY_TYPE_STR) || !ds.contains(KEY_ACTION_STR) || !ds.contains(KEY_PARAM_STR))
	{
		response->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		response->set(KEY_DETAIL_STR, "101");
		return false;
	}
	if(ds[KEY_TYPE_STR].toString() != TYPE_REQUEST_STR)
	{
		response->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		response->set(KEY_DETAIL_STR, "102");
		return false;
	}
	Dynamic::Var var = ds[KEY_PARAM_STR];
	DynamicStruct param;
	try
	{
		param = var.extract<DynamicStruct>();
	}
	catch(Exception& e)
	{
		response->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		response->set(KEY_DETAIL_STR, "103");
		return false;
	}
	if(!param.contains(REG_UUID_STR))
	{
		response->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		response->set(KEY_DETAIL_STR, "104");
		return false;
	}
	return true;
}

bool CHTTPRequestHandler::parseAction(std::string action, std::string& component, std::string& method)
{
	std::string::size_type pos;
	pos = action.find(".");
	if(pos == std::string::npos)
	{
		return false;
	}
	component = action.substr(0, pos);
	method = action.substr(pos + 1, action.length() - pos - 1);
	return true;
}

void CHTTPRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setContentType("application/json");
	response.setChunkedTransferEncoding(true);
	if(m_buf == NULL)
		m_buf = new char[512];
	memset(m_buf, 0, 512);
	request.stream().getline(m_buf, 512, '\n');
	infof("%s, %d: Receive HTTP request[%s]", __FILE__, __LINE__, m_buf);
	JSON::Parser parser;
	Dynamic::Var var;
	try
	{
		var = parser.parse(m_buf);
	}
	catch(Exception& e)
	{
		JSON::Object::Ptr re = new JSON::Object;
		re->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		re->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		re->set(KEY_DETAIL_STR, "100");
		DynamicStruct ds_res = *re;
		response.sendBuffer(ds_res.toString().c_str(), ds_res.toString().length());
		infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, ds_res.toString().c_str());
		re = NULL;
		return;
	}
	JSON::Object::Ptr obj = var.extract<JSON::Object::Ptr>();
	JSON::Object::Ptr res = new JSON::Object(*obj);
	if(!checkRequestFormat(obj, res))
	{
		DynamicStruct ds_res = *res;
		response.sendBuffer(ds_res.toString().c_str(), ds_res.toString().length());
		infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, ds_res.toString().c_str());
		res = NULL;
		return;
	}
	DynamicStruct ds = *obj;
	std::string uuid = ds[KEY_PARAM_STR][REG_UUID_STR].toString();
	std::string action = ds[KEY_ACTION_STR].toString();
	std::string component = "";
	std::string method = "";
	bool ret = parseAction(action, component, method);
	if(!ret)
	{
		res->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		res->set(KEY_DETAIL_STR, "105");
		DynamicStruct ds_res = *res;
		response.sendBuffer(ds_res.toString().c_str(), ds_res.toString().length());
		infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, ds_res.toString().c_str());
		res = NULL;
		return;
	}
	if(component == COMPONENT_SERVER_STR)
	{
		if(method == SERVER_METHOD_CHECK)
		{
			res->set(KEY_RESULT_STR, RESULT_GOOD_STR);
			CDeviceManager* dev_mgr = CDeviceManager::instance();
			DeviceInfo* dev_info = dev_mgr->getDevice(uuid);
			DynamicStruct param;
			param[REG_UUID_STR] = uuid;
			if(dev_info != NULL)
			{
				param[REG_STATE_STR] = "online";
				param[REG_DEV_TYPE_STR] = dev_info->devType;
			}
			else
			{
				param[REG_STATE_STR] = "offline";
			}
			res->remove(KEY_PARAM_STR);
			res->set(KEY_PARAM_STR, param);
		}
		else
		{
			res->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			res->set(KEY_DETAIL_STR, "106");
		}
		DynamicStruct ds_res = *res;
		infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, ds_res.toString().c_str());
		response.sendBuffer(ds_res.toString().c_str(), ds_res.toString().length());
		res = NULL;
		return;
	}
	RequestInfo* req = new RequestInfo((UInt64)this, uuid, 5*1000*1000, obj);
	CRegServer* reg_server = CRegServer::instance();
	//it will hang until response send back, or timeout
	reg_server->sendRequest(req);
	res = req->response;
	if(res.isNull())
	{
		warnf("%s, %d: Request timeout.", __FILE__, __LINE__);
		DynamicStruct result = *obj;
		result[KEY_RESULT_STR] = RESULT_FAIL_STR;
		result[KEY_DETAIL_STR] = "timeout";
		infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, result.toString().c_str());
		response.sendBuffer(result.toString().c_str(), result.toString().length());
	}
	else
	{
		DynamicStruct result = *res;
		infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, result.toString().c_str());
		response.sendBuffer(result.toString().c_str(), result.toString().length());
	}
	res = NULL;
	delete req;
}


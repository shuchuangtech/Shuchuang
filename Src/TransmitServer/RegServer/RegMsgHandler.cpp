#include "TransmitServer/RegServer/RegMsgHandler.h"
#include "TransmitServer/DeviceManager.h"
#include "Common/RPCDef.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/TaskNotification.h"
#include "Poco/MD5Engine.h"
#include "Common/PrintLog.h"
CRegMsgHandler::CRegMsgHandler(int type)
:Task("RegMsgHandler")
{
	m_type = type;
	m_receive = false;
	m_req_id = 0;
	m_http_response = NULL;
}

CRegMsgHandler::~CRegMsgHandler()
{
}

bool CRegMsgHandler::receiveBytes(Timespan timeout)
{
	StreamSocket ss = m_socket->socket;
	char buf[512] = {0, };
	ss.setReceiveTimeout(timeout);
	try
	{
		if(ss.receiveBytes(buf, 511) <= 0)
		{
			if(m_type == 1 && m_socket->state == SocketTime::Connected)
			{
				CDeviceManager* dm = CDeviceManager::instance();
				UInt64 id = (UInt64)ss.impl();
				dm->deviceOffline(id);
			}
			m_socket->state = SocketTime::Disconnected;
			infof("%s, %d: Client [%s] disconnected.", __FILE__, __LINE__, m_socket->saddr.toString().c_str());
			return false;
		}
		else
		{
			m_recv_buf += buf;
			//maybe more data
			while(ss.available())
			{
				memset(buf, 0, 512);
				if(ss.receiveBytes(buf, 511) > 0)
				{
					m_recv_buf += buf;
					tracepoint();
				}
			}
		}
	}
	catch(Exception& e)
	{
		if(m_type == 0)
		{
			warnf("%s, %d: Ssl register receive timeout.", __FILE__, __LINE__);
			m_socket->state = SocketTime::Disconnected;
		}
		else if(m_socket->state == SocketTime::Connecting)
		{
			warnf("%s, %d: Reg register receive timeout.", __FILE__, __LINE__);
			m_socket->state = SocketTime::Disconnected;
		}
		return false;
	}
	tracef("%s, %d: Receive buf: %s", __FILE__, __LINE__, m_recv_buf.c_str());
	m_socket->state = SocketTime::Connected;
	return true;
}

bool CRegMsgHandler::handleSslMsg(JSON::Object::Ptr request, JSON::Object::Ptr result)
{
	DynamicStruct param;
	if(!formatCheck(request, result, param))
	{
		warnf("%s, %d: Request format error.", __FILE__, __LINE__);
		return false;
	}
	DynamicStruct ds = *request;
	std::string action = ds[KEY_ACTION_STR].toString();
	std::string component = "";
	std::string method = "";
	if(!parseAction(action, component, method))
	{
		warnf("%s, %d: Request action format error.", __FILE__, __LINE__);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "105");
		return false;
	}
	if(component != COMPONENT_SERVER_STR)
	{
		warnf("%s, %d: Component error.", __FILE__, __LINE__);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "200");
		return false;
	}
	if(method != SERVER_METHOD_TOKEN)
	{
		warnf("%s, %d: Method error.", __FILE__, __LINE__);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "201");
		return false;
	}	
	if(!param.contains(REG_UUID_STR) || !param.contains(REG_DEV_TYPE_STR) || !param.contains(REG_DEV_NAME_STR) || !param.contains(REG_DEV_MANU_STR) || !param.contains(REG_MOBILETOKEN_STR))
	{
		warnf("%s, %d: Param missing.", __FILE__, __LINE__);
		result->set(KEY_DETAIL_STR, "104");
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		return false;
	}
	std::string token = "";
	std::string dev_uuid = param[REG_UUID_STR].toString();
	std::string dev_type = param[REG_DEV_TYPE_STR].toString();
	std::string dev_name = param[REG_DEV_NAME_STR].toString();
	std::string mobile_token = param[REG_MOBILETOKEN_STR].toString();
	Timestamp now;
	Int64 tms = now.epochMicroseconds();
	char tms_str[32];
	snprintf(tms_str, 31, "%lld", tms);
	std::string key = "alpha2015";
	key += tms_str;
	MD5Engine md5;
	md5.update(key);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key = DigestEngine::digestToHex(digest);
	CDeviceManager* dm = CDeviceManager::instance();
	if(dm->addDevice(dev_uuid, (UInt64)m_socket->socket.impl(), dev_type, token, mobile_token))
	{
		result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		result->remove(KEY_PARAM_STR);
		DynamicStruct returnParam;
		returnParam[REG_UUID_STR] = dev_uuid;
		returnParam[REG_TOKEN_STR] = token;
		returnParam[REG_TIMESTAMP_STR] = tms_str;
		returnParam[REG_KEY_STR] = md5key;
		result->set(KEY_PARAM_STR, returnParam);
		return true;
	}
	else
	{
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		return false;
	}
}

bool CRegMsgHandler::handleRegMsg(JSON::Object::Ptr request, JSON::Object::Ptr result)
{
	DynamicStruct ds = *request;
	tracef("%s, %d: handle reg msg %s", __FILE__, __LINE__, ds.toString().c_str());
	if(ds.contains(KEY_TYPE_STR) && ds[KEY_TYPE_STR] == TYPE_RESPONSE_STR)
	//response to http request
	{
		if(ds.contains(KEY_REQUEST_ID_STR))
			m_req_id = ds[KEY_REQUEST_ID_STR];
		else
			m_req_id = 1;
		m_http_response = request;
		return true;
	}
	//
	DynamicStruct param;
	if(!formatCheck(request, result, param))
	{
		warnf("%s, %d: Request format error.", __FILE__, __LINE__);
		return false;
	}
	std::string action = ds[KEY_ACTION_STR].toString();
	std::string component = "";
	std::string method = "";
	result->set(KEY_ACTION_STR, action);
	if(!parseAction(action, component, method))
	{
		warnf("%s, %d: Request action format error.", __FILE__, __LINE__);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "105");
		return false;
	}
	if(component != COMPONENT_SERVER_STR)
	{
		warnf("%s, %d: Component error.", __FILE__, __LINE__);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "200");
		return false;
	}
	if(method == SERVER_METHOD_REGISTER)
	{
		if(!param.contains(REG_TOKEN_STR) || !param.contains(REG_UUID_STR))
		{
			warnf("%s, %d: Param missing.", __FILE__, __LINE__);
			result->set(KEY_DETAIL_STR, "104");
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return false;
		}
		std::string token = param[REG_TOKEN_STR].toString();
		std::string dev_uuid = param[REG_UUID_STR].toString();
		CDeviceManager* dm = CDeviceManager::instance();
		if(dm->deviceOnline(dev_uuid, token, (UInt64)m_socket->socket.impl()))
		{
			result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		}
		else
		{
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		}
	}
	else if(method == SERVER_METHOD_KEEPALIVE)
	{
		if(!param.contains(REG_UUID_STR))
		{
			warnf("%s, %d: Param missing.", __FILE__, __LINE__);
			result->set(KEY_DETAIL_STR, "104");
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return false;
		}
		std::string dev_uuid = param[REG_UUID_STR].toString();
		CDeviceManager* dm = CDeviceManager::instance();
		dm->keepAliveDevice(dev_uuid);
		result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
	}
	else if(method == SERVER_METHOD_BIND)
	{
		if(!param.contains(REG_MOBILETOKEN_STR))
		{
			warnf("%s, %d: Param missing.", __FILE__, __LINE__);
			result->set(KEY_DETAIL_STR, 104);
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return false;
		}
		std::string dev_uuid = param[REG_UUID_STR].toString();
		std::string mobile_token = param[REG_MOBILETOKEN_STR];
		CDeviceManager* dm = CDeviceManager::instance();
		if(dm->bindMobile(dev_uuid, mobile_token))
		{
			result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		}
		else
		{
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		}
		return true;
	}
	else if(method == SERVER_METHOD_UNBIND)
	{
		std::string dev_uuid = param[REG_UUID_STR].toString();
		CDeviceManager* dm = CDeviceManager::instance();
		if(dm->unbindMobile(dev_uuid))
		{
			result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		}
		else
		{
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		}
		return true;
	}
	else
	{
		warnf("%s, %d: Method error.", __FILE__, __LINE__);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "201");
		return false;
	}
	return true;
}

bool CRegMsgHandler::parseRequest(const char* buf, JSON::Object::Ptr& request)
{
	JSON::Parser parser;
	try
	{
		Dynamic::Var var = parser.parse(buf);
		request = var.extract<JSON::Object::Ptr>();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: RegMsgHandler parse request fail.", __FILE__, __LINE__);
		return false;
	}
	return true;
}

void CRegMsgHandler::runTask()
{
	m_recv_buf = "";
	if(!receiveBytes(Timespan(20, 0)))
	{
		m_receive = false;
		return;
	}
	m_receive = true;
	JSON::Object::Ptr request;
	JSON::Object::Ptr result;
	if(!parseRequest(m_recv_buf.c_str(), request))
	{
		result = new JSON::Object;
		result->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "100");
	}
	else
	{
		result = new JSON::Object(request);
		if(m_type == 0)
			//ssl
		{
			handleSslMsg(request, result);
		}
		else if(m_type == 1)
			//reg
		{
			handleRegMsg(request, result);
		}
	}
	m_recv_buf = "";
	if(m_req_id == 0)
	{
		DynamicStruct ds = *result;
		int ret = m_socket->socket.sendBytes(ds.toString().c_str(), ds.toString().length());
		tracef("%s, %d: result %s, send bytes: %d.", __FILE__, __LINE__, ds.toString().c_str(), ret);
	}
	return;
}

bool CRegMsgHandler::formatCheck(JSON::Object::Ptr request, JSON::Object::Ptr result, DynamicStruct& param)
{
	DynamicStruct ds = *request;
	tracef("%s, %d: request: %s.", __FILE__, __LINE__, ds.toString().c_str());
	if(!ds.contains(KEY_TYPE_STR) || !ds.contains(KEY_ACTION_STR) || !ds.contains(KEY_PARAM_STR))
	{
		result->set(KEY_DETAIL_STR, "101");
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		return false;
	}
	if(ds[KEY_TYPE_STR].toString() != TYPE_REQUEST_STR)
	{
		result->set(KEY_DETAIL_STR, "102");
		result->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		return false;
	}	
	result->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
	Dynamic::Var var = ds[KEY_PARAM_STR];
	try
	{
		param = var.extract<DynamicStruct>();
	}
	catch(Exception& e)
	{
		result->set(KEY_DETAIL_STR, "103");
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		return false;
	}
	return true;
}

bool CRegMsgHandler::parseAction(std::string action, std::string& component, std::string& method)
{
	std::string::size_type pos;
	pos = action.find(".");
	if(pos == std::string::npos)
		return false;
	component = action.substr(0, pos);
	method = action.substr(pos + 1, action.length() - pos - 1);
	return true;
}

UInt64 CRegMsgHandler::getRequestID()
{
	return m_req_id;
}

bool CRegMsgHandler::socketReceive()
{
	return m_receive;
}

JSON::Object::Ptr CRegMsgHandler::getHTTPResponse()
{
	return m_http_response;
}

int CRegMsgHandler::getType()
{
	return m_type;
}

bool CRegMsgHandler::setSocket(SocketTime* st)
{
	m_socket = st;
	return true;
}

SocketTime* CRegMsgHandler::getSocket()
{
	return m_socket;
}


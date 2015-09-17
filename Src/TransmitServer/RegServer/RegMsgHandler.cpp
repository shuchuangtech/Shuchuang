#include "TransmitServer/RegMsgHandler.h"
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
	m_buf = NULL;
	m_receive = false;
	m_req_id = 0;
	m_http_response = NULL;
}

CRegMsgHandler::~CRegMsgHandler()
{
	if(m_buf != NULL)
	{
		delete m_buf;
		m_buf = NULL;
	}
}

bool CRegMsgHandler::receiveBytes(char* buf, int length, Timespan timeout)
{
	StreamSocket ss = m_socket->socket;
	if(ss.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
	{
		ss.setReceiveTimeout(timeout);
		try
		{
			if(ss.receiveBytes(buf, length) <= 0)
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
	}
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
	if(!param.contains(PARAM_UUID_STR) || !param.contains(PARAM_DEV_TYPE_STR) || !param.contains(PARAM_DEV_NAME_STR) || !param.contains(PARAM_TIMESTAMP_STR) || !param.contains(PARAM_KEY_STR))
	{
		warnf("%s, %d: Param missing.", __FILE__, __LINE__);
		result->set(KEY_DETAIL_STR, "104");
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		return false;
	}
	std::string token = "";
	std::string dev_uuid = param[PARAM_UUID_STR].toString();
	std::string dev_type = param[PARAM_DEV_TYPE_STR].toString();
	std::string dev_name = param[PARAM_DEV_NAME_STR].toString();
	std::string timestamp = param[PARAM_TIMESTAMP_STR].toString();
	std::string key = param[PARAM_KEY_STR].toString();
	if(!verifyKey(timestamp, SERVER_KEY_STR, key))
	{
		warnf("%s, %d: Device %s verify failed.", __FILE__, __LINE__, dev_uuid.c_str());
		result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		result->set(KEY_DETAIL_STR, "300");
		return false;
	}
	CDeviceManager* dm = CDeviceManager::instance();
	if(dm->addDevice(dev_uuid, (UInt64)m_socket->socket.impl(), dev_type, token))
	{
		result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		result->remove(KEY_PARAM_STR);
		DynamicStruct returnParam;
		returnParam[PARAM_UUID_STR] = dev_uuid;
		returnParam[PARAM_TOKEN_STR] = token;
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
	if(ds.contains(KEY_TYPE_STR) && ds[KEY_TYPE_STR] == TYPE_RESPONSE_STR)
			//response to http request
	{
		if(ds.contains(KEY_REQUEST_ID_STR))
			m_req_id = ds[KEY_REQUEST_ID_STR];
		else
			m_req_id = -1;
		m_http_response = request;
		return true;
	}
	DynamicStruct param;
	if(!formatCheck(request, result, param))
	{
		warnf("%s, %d: Request format error.", __FILE__, __LINE__);
		return false;
	}
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
	if(method == SERVER_METHOD_REGISTER)
	{
		if(!param.contains(PARAM_TOKEN_STR) || !param.contains(PARAM_UUID_STR))
		{
			warnf("%s, %d: Param missing.", __FILE__, __LINE__);
			result->set(KEY_DETAIL_STR, "104");
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return false;
		}
		std::string token = param[PARAM_TOKEN_STR].toString();
		std::string dev_uuid = param[PARAM_UUID_STR].toString();
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
		if(!param.contains(PARAM_UUID_STR))
		{
			warnf("%s, %d: Param missing.", __FILE__, __LINE__);
			result->set(KEY_DETAIL_STR, "104");
			result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return false;
		}
		std::string dev_uuid = param[PARAM_UUID_STR].toString();
		CDeviceManager* dm = CDeviceManager::instance();
		dm->keepAliveDevice(dev_uuid);
		result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
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

bool CRegMsgHandler::parseRequest(char* buf, JSON::Object::Ptr& request)
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
	if(m_buf != NULL)
	{
		delete m_buf;
		m_buf = NULL;
	}
	m_buf = new char[512];
	memset(m_buf, 0, 512);
	if(!receiveBytes(m_buf, 512, Timespan(20, 0)))
	{
		m_receive = false;
		delete[] m_buf;
		m_buf = NULL;
		return;
	}
	m_receive = true;
	JSON::Object::Ptr request;
	JSON::Object::Ptr result;
	if(!parseRequest(m_buf, request))
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
	delete[] m_buf;
	m_buf = NULL;
	if(m_req_id == 0)
	{
		DynamicStruct ds = *result;
		int ret = m_socket->socket.sendBytes(ds.toString().c_str(), ds.toString().length());
		tracef("%s, %d: result %s, send bytes: %d.", __FILE__, __LINE__, ds.toString().c_str(), ret);
	}
	return;
}

bool CRegMsgHandler::verifyKey(std::string timestamp, std::string skey, std::string key)
{
	MD5Engine md5;
	std::string s1 = skey + timestamp;
	md5.update(s1);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key(DigestEngine::digestToHex(digest));
	if(md5key == key)
	{
		return true;
	}
	else
	{
		return false;
	}
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


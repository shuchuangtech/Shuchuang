#include "TransmitServer/RegMsgHandler.h"
#include "TransmitServer/DeviceManager.h"
#include "Common/RegKey.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/TaskNotification.h"
#include "Common/PrintLog.h"
CRegMsgHandler::CRegMsgHandler(int type)
:Task("SslMsgHandler")
{
	m_type = type;
	m_buf = NULL;
	m_socket = NULL;
}

CRegMsgHandler::~CRegMsgHandler()
{
	if(m_buf != NULL)
	{
		delete m_buf;
		m_buf = NULL;
	}
	if(m_socket != NULL)
	{
		delete m_socket;
		m_socket = NULL;
	}
}

void CRegMsgHandler::runTask()
{
	if(m_result.isNull())
		m_result = new JSON::Object;
	if(m_type == 0)
		//ssl
	{
		if(m_buf == NULL)
		{
			m_result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return;
		}
		JSON::Parser parser;
		Dynamic::Var var = parser.parse(m_buf);
		m_param = var.extract<JSON::Object::Ptr>();
		DynamicStruct ds = *m_param;
		delete[] m_buf;
		m_buf = NULL;
		if(!ds.contains(KEY_ACTION_STR) || !ds.contains(KEY_UUID_STR) || !ds.contains(KEY_KEY_STR) || ds[KEY_ACTION_STR].toString() != ACTION_GETTOKEN_STR)
		{
			m_result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return;
		}
		//postNotification(new TaskFinishedNotification(this));
		std::string token = "";
		std::string dev_uuid = ds[KEY_UUID_STR].toString();
		std::string dev_type = ds[KEY_DEVTYPE_STR].toString();
		CDeviceManager* dm = CDeviceManager::instance();
		if(dm->addDevice(dev_uuid, m_id, dev_type, token))
		{
			m_result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		}
		else
		{
			m_result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		}
		m_result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		m_result->set(KEY_TOKEN_STR, token);
	}
	else if(m_type == 1)
		//reg
	{
		if(m_buf == NULL)
		{
			m_result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			tracepoint();
			return;
		}
		JSON::Parser parser;
		Dynamic::Var var = parser.parse(m_buf);
		m_param = var.extract<JSON::Object::Ptr>();
		DynamicStruct ds = *m_param;
		delete[] m_buf;
		m_buf = NULL;
		if(ds.contains(KEY_TYPE_STR) && ds[KEY_TYPE_STR] == TYPE_RESPONSE_STR)
			//response to http request
		{
			m_result = NULL;
			m_result = m_param;
			return;
		}
		if(!ds.contains(KEY_ACTION_STR) || !ds.contains(KEY_UUID_STR))
		{
			m_result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			return;
		}
		std::string action = ds[KEY_ACTION_STR].toString();
		std::string token = "";
		if(ds.contains(KEY_TOKEN_STR))
		{
			token = ds[KEY_TOKEN_STR].toString();
		}
		std::string dev_uuid = ds[KEY_UUID_STR].toString();
		CDeviceManager* dm = CDeviceManager::instance();
		m_result->set(KEY_ACTION_STR, action);
		m_result->set(KEY_UUID_STR, dev_uuid);
		if(action == ACTION_REGISTER_STR)
		{
			if(dm->deviceOnline(dev_uuid, token, m_id))
			{
				m_result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
			}
			else
			{
				m_result->set(KEY_RESULT_STR, RESULT_FAIL_STR);
			}
		}
		else if(action == ACTION_KEEPALIVE_STR)
		{
			dm->keepAliveDevice(dev_uuid);
			m_result->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		}
	}
	else
	{
		return;
	}
}

int CRegMsgHandler::getId()
{
	return m_id;
}

JSON::Object::Ptr CRegMsgHandler::getResult()
{
	return m_result;
}

bool CRegMsgHandler::setParam(UInt64 id, const char* param, int paramLenth, StreamSocket sock)
{
	m_id = id;
	m_buf = new char[512];
	memset(m_buf, 0, 512);
	memcpy(m_buf, param,  paramLenth > 512?512:paramLenth);
	m_socket = new StreamSocket(sock);
	return true;
}


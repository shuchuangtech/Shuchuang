#include "Device/RPCClient.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Device/Component/UserManager.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/TaskNotification.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
using namespace Poco;
CRPCClient::CRPCClient()
:Task("RPCClient")
{
	m_request = NULL;
	m_result = NULL;
}

CRPCClient::~CRPCClient()
{
}

RequestNotification::Ptr CRPCClient::getResult()
{
	return m_result;
}

bool CRPCClient::setRequest(RequestNotification::Ptr request)
{
	if(request.isNull())
		return false;
	m_request = request;
	m_param = m_request->getParam();
	m_id = m_request->getID();
	DynamicStruct ds = *m_param;
	tracef("%s, %d: RPC Client receive request[%s].\n", __FILE__, __LINE__, ds.toString().c_str());
	m_request = NULL;
	return true;
}

bool CRPCClient::parseAction(std::string& opt, std::string& component, std::string& method)
{
	std::string::size_type pos;
	pos = opt.find(".");
	if(pos == std::string::npos)
	{
		warnf("%s, %d: Action parse failed.\n", __FILE__, __LINE__);
		return false;
	}
	component = opt.substr(0, pos);
	method = opt.substr(pos + 1, opt.length() - pos - 1);
	return true;
}

void CRPCClient::runTask()
{
	tracef("%s, %d: RPC Client runTask().\n", __FILE__, __LINE__);
	DynamicStruct request = *(m_param);
	std::string opt = request[KEY_ACTION_STR];
	std::string component = "";
	std::string method = "";
	bool result = false;
	if(!parseAction(opt, component, method))
	{
		return;
	}
	tracef("%s, %d: opt:%s, component:%s, method:%s.\n", __FILE__, __LINE__, opt.c_str(), component.c_str(), method.c_str());
	JSON::Parser parser;
	Dynamic::Var temp = parser.parse(request[KEY_PARAM_STR].toString().c_str());
	JSON::Object::Ptr param = temp.extract<JSON::Object::Ptr>();
	std::string detail = "";
	if(component == COMPONENT_USER_STR)
	{
		CUserManager* user = CUserManager::instance();
		if(method == USER_METHOD_LOGIN)
		{
			if(user->login(param, detail))
				result = true;
		}
		else if(method == USER_METHOD_LOGOUT)
		{
			if(user->logout(param))
				result = true;
		}
		else if(method == USER_METHOD_PASSWD)
		{
			if(user->passwd(param, detail))
				result = true;
		}
	}
	/*
	else if(component == COMPONENT_CONN_STR)
	{
		if(m_session_manager == NULL)
			m_session_manager = CSessionManager::instance();
		if(method == CONN_METHOD_CONNECT)
		{
			if(m_session_manager->connect(&param))
				result = true;
		}
		else if(method == CONN_METHOD_DISCONNECT)
		{
			if(m_session_manager->disconnect(&param))
				result = true;
		}
	}
	*/
	else
	{
		warnf("%s, %d: Invalid component.\n", __FILE__, __LINE__);
	}
	if(result)
	{
		request[KEY_RESULT_STR] = RESULT_GOOD_STR;
	}
	else
	{
		request[KEY_RESULT_STR] = RESULT_FAIL_STR;
		request[KEY_DETAIL_STR] = detail;
	}
	request[KEY_TYPE_STR] = TYPE_RESPONSE_STR;
	request.erase(KEY_PARAM_STR);
	request[KEY_PARAM_STR] = *param;
	tracef("%s, %d: Finish request:[%s].\n", __FILE__, __LINE__, request.toString().c_str());
	if(!m_result.isNull())
	{
		m_result = NULL;
	}
	parser.reset();
	Dynamic::Var var = parser.parse(request.toString().c_str());
	JSON::Object::Ptr pResult = var.extract<JSON::Object::Ptr>();
	m_result = new RequestNotification(m_id, pResult);
	//postNotification(new TaskFinishedNotification(this));
}


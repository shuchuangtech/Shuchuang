#include "Device/RPCClient.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Device/Component/User/UserManager.h"
#include "Device/Component/Task/TaskManager.h"
#include "Device/Component/DeviceController.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/TaskNotification.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
using namespace Poco;
CRPCClient::CRPCClient()
:Task("RPCClient")
{
	m_response = NULL;
}

CRPCClient::~CRPCClient()
{
}

JSON::Object::Ptr& CRPCClient::getResponse()
{
	return m_response;
}

bool CRPCClient::setRequest(std::string& request)
{
	m_request_str = request;
	return true;
}

bool CRPCClient::setID(UInt64 id)
{
	m_id = id;
	return true;
}

UInt64 CRPCClient::getID()
{
	return m_id;
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
	//when use goto, definition must at top, can't cross
	JSON::Object::Ptr pResult = new JSON::Object;
	pResult->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
	bool result = false;
	std::string detail = "";
	JSON::Parser parser;
	Dynamic::Var var;

	//convert request string to JSON format
	JSON::Object::Ptr request;
	//parse request
	std::string type = "";
	std::string opt = "";
	std::string param_str = "";
	std::string component = "";
	std::string method = "";

	//convert param string to JSON format
	Dynamic::Var temp;
	JSON::Object::Ptr param;

	CUserManager* user = NULL;
	CTaskManager* task = NULL;
	CDeviceController* device = NULL;

	std::string token = "";
	try
	{
		var = parser.parse(m_request_str.c_str());
	}
	catch(Exception& e)
	{
		detail = "100";
		result = false;
		goto done;
	}
	request = var.extract<JSON::Object::Ptr>();
	if(!request->has(KEY_TYPE_STR) || !request->has(KEY_ACTION_STR) || !request->has(KEY_PARAM_STR))
	{
		detail = "101";
		result = false;
		goto done;
	}
	type = request->getValue<std::string>(KEY_TYPE_STR);;
	opt = request->getValue<std::string>(KEY_ACTION_STR);
	param_str = request->getValue<std::string>(KEY_PARAM_STR);
	component = "";
	method = "";
	if(!parseAction(opt, component, method))
	{
		detail = "105";
		result = false;
		goto done;
	}
	tracef("%s, %d: opt:%s, component:%s, method:%s.\n", __FILE__, __LINE__, opt.c_str(), component.c_str(), method.c_str());
	parser.reset();
	try{
		temp = parser.parse(param_str.c_str());
	}
	catch(Exception& e)
	{
		detail = "103";
		result = false;
		goto done;
	}
	param = temp.extract<JSON::Object::Ptr>();
	if(component == COMPONENT_USER_STR)
	{
		if(user == NULL)
			user = CUserManager::instance();
		if(method == USER_METHOD_LOGIN)
		{
			if(user->login(param, detail))
				result = true;
		}
		else if(method == USER_METHOD_LOGOUT)
		{
			if(user->logout(param, detail))
				result = true;
		}
		else if(method == USER_METHOD_PASSWD)
		{
			if(user->passwd(param, detail))
				result = true;
		}
		else
		{
			result = false;
			detail = "201";
		}
	}
	else if(component == COMPONENT_TASK_STR)
	{
		if(!param->has(KEY_TOKEN_STR))
		{
			detail = "401";
			result = false;
			goto done;
		}
		token = param->getValue<std::string>(KEY_TOKEN_STR);
		if(user == NULL)
			user = CUserManager::instance();
		if(!user->verifyUser(token))
		{
			detail = "402";
			result = false;
			goto done;
		}
		if(task == NULL)
			task = CTaskManager::instance();
		if(method == TASK_METHOD_ADD)
		{
			if(task->addTask(param, detail))
				result = true;
		}
		else if(method == TASK_METHOD_REMOVE)
		{
			if(task->removeTask(param, detail))
				result = true;
		}
		else if(method == TASK_METHOD_MODIFY)
		{
			if(task->modifyTask(param, detail))
				result = true;
		}
		else if(method == TASK_METHOD_GETTASKS)
		{
			if(task->getTasks(param, detail))
				result = true;
		}
		else
		{
			result = false;
			detail = "201";
			goto done;
		}
	}
	else if(component == COMPONENT_DEVICE_STR)
	{
		if(device == NULL)
			device = CDeviceController::instance();
		if(method == DEVICE_METHOD_OPEN)
		{
			if(device->openDoor(param, detail))
				result = true;
		}
		else if(method == DEVICE_METHOD_CLOSE)
		{
			if(device->closeDoor(param, detail))
				result = true;
		}
		else if(method == DEVICE_METHOD_CHECK)
		{
			if(device->checkDoor(param, detail))
				result = true;
		}
		else
		{
			result = false;
			detail = "201";
			goto done;
		}
	}
	else
	{
		warnf("%s, %d: Invalid component.\n", __FILE__, __LINE__);
		result = false;
		detail = "200";
		goto done;
	}
done:
	if(result)
	{
		pResult->set(KEY_RESULT_STR, RESULT_GOOD_STR);
		pResult->set(KEY_PARAM_STR, param);
	}
	else
	{
		pResult->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		pResult->set(KEY_DETAIL_STR, detail);
	}
	DynamicStruct ds_request = *request;
	tracef("%s, %d: Finish request:[%s].\n", __FILE__, __LINE__, ds_request.toString().c_str());
	if(!m_response.isNull())
		m_response = NULL;
	m_response = pResult;
	//postNotification(new TaskFinishedNotification(this));
}


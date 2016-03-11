#ifndef __REG_PROXY_H__
#define __REG_PROXY_H__
#include "Poco/Types.h"
#include "Poco/Timer.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Timestamp.h"
#include "Device/Notification/RequestNotification.h"
#include "Poco/NotificationCenter.h"
#include "Poco/SingletonHolder.h"
#include "Device/RPC/RPCServer.h"
class CRegProxy
{
public:
#define  ERROR_CHOICE int
	enum REQUEST_ACTION
	{
		ACTION_GETTOKEN = 0,
		ACTION_REGISTER,
		ACTION_KEEPALIVE,
	};
	enum
	{
		PLAIN_SOCKET = 0x0001,
		SECURE_SOCKET = 0x0002,
	};
	static CRegProxy* instance()
	{
		static Poco::SingletonHolder<CRegProxy> sh;
		return sh.get();
	}
	CRegProxy();
	~CRegProxy();
	void start();
	void stop();
	void handleNf(RequestNotification* pNf);
private:
	bool registerToServer();
	bool getRegisterToken();
	void dealError(ERROR_CHOICE);
	void createPacket(char* buf, Poco::UInt16 size, REQUEST_ACTION);
	bool sendKeepAlive();
	bool parseAction(std::string& action, std::string& component, std::string& method);
	//bool handleRequest(DynamicStruct* param);

	void							onTimer(Poco::Timer& timer);
	bool							m_started;
	Poco::Timer						m_timer;
	std::string						m_ssl_host;
	Poco::UInt16					m_ssl_port;
	std::string						m_reg_host;
	Poco::UInt16					m_reg_port;
	std::string						m_uuid;
	std::string						m_dev_name;
	std::string						m_dev_type;
	std::string						m_manufacture;
	Poco::Timestamp::TimeDiff		m_checkPeriod;
	Poco::Timestamp::TimeDiff		m_keepAliveTimeout;
	Poco::Timestamp					m_lastCheckTime;
	bool							m_lastRegState;
	bool							m_serverKeepAlive;
	Poco::Net::SecureStreamSocket*	m_sock;
	Poco::Net::SecureStreamSocket*	m_ssl_sock;
	std::string						m_token;
	CRPCServer*						m_rpc;
};
#endif


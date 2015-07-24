#ifndef __REG_PROXY_H__
#define __REG_PROXY_H__
#include "Poco/Types.h"
#include "Poco/Timer.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Timestamp.h"
#include "Device/RequestNotification.h"
#include "Poco/NotificationCenter.h"
#include "Poco/SingletonHolder.h"
#include "Device/RPCServer.h"
using namespace Poco;
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
		static SingletonHolder<CRegProxy> sh;
		return sh.get();
	}
	CRegProxy(); 
	~CRegProxy();
	void setSecureServerInfo(std::string ssl_host, UInt16 ssl_port);
	void setServerInfo(std::string reg_host, UInt16 reg_port);
	void start();
	void stop();
	void handleNf(RequestNotification* pNf);
private:
	bool registerToServer();
	bool getRegisterToken();
	void dealError(ERROR_CHOICE);
	void createPacket(char* buf, UInt16 size, REQUEST_ACTION);
	bool sendKeepAlive();
	bool parseAction(std::string& action, std::string& component, std::string& method);
	//bool handleRequest(DynamicStruct* param);

	void onTimer(Timer& timer);
	bool						m_started;
	Timer						m_timer;
	std::string					m_ssl_host;
	UInt16						m_ssl_port;
	std::string					m_reg_host;
	UInt16						m_reg_port;
	Timestamp::TimeDiff			m_checkPeriod;
	Timestamp::TimeDiff			m_keepAliveTimeout;
	Timestamp					m_lastCheckTime;
	bool						m_lastRegState;
	bool						m_serverKeepAlive;
	Net::StreamSocket*			m_sock;
	Net::SecureStreamSocket*	m_ssl_sock;
	std::string					m_token;
	CRPCServer*					m_rpc;
};
#endif


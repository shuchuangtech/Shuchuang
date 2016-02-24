#include "Common/ConfigManager.h"
#include "Common/PrintLog.h"
#include "Device/Network/NetworkManager.h"
int main(int argc, char** argv)
{
	std::string configPath = "";
	if(argc >= 2)
	{
		configPath = argv[1];
	}
	else
	{
#ifdef __SC_ARM__
		configPath = "/mnt/nand1-1/Application/config";
#else
		printf("argc < 2.");
		return 0;
#endif
	}
	initPrintLogger("");
	CConfigManager* config = CConfigManager::instance();
	config->init(configPath.c_str());
	CNetworkManager* network = CNetworkManager::instance();
	network->initIf("eth0");
	return 0;
}


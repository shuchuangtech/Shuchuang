#include "Device/Network/NetworkManager.h"
#include "Common/ConfigManager.h"
int main()
{
	CConfigManager* config = CConfigManager::instance();
	config->init("./config");
	CNetworkManager* network = CNetworkManager::instance();
	network->initIf("eth0");
	return 0;
}

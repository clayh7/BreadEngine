#include "Engine/NetworkSystem/BNetworkSystem.hpp"

#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC size_t BNetworkSystem::GAME_PORT = 4334;
STATIC char const * BNetworkSystem::EVENT_NETWORK_STARTUP = "NetworkStartup";
STATIC char const * BNetworkSystem::EVENT_NETWORK_SHUTDOWN = "NetworkShutdown";
STATIC char const * BNetworkSystem::EVENT_NETWORK_UPDATE = "NetworkUpdateEvent";


//-------------------------------------------------------------------------------------------------
STATIC bool BNetworkSystem::Startup()
{
	WSADATA wsa_data;
	//version 2.2
	WORD version = MAKEWORD(2, 2);
	int error = WSAStartup(version, &wsa_data);
	if(error == SOCKET_ERROR)
	{
		NetworkUtils::ReportError();
	}

	BEventSystem::RegisterEvent(EVENT_ENGINE_UPDATE, &BNetworkSystem::OnUpdate, -10);
	BEventSystem::TriggerEvent(EVENT_NETWORK_STARTUP);

	return true;
}


//-------------------------------------------------------------------------------------------------
STATIC void BNetworkSystem::Shutdown()
{
	BEventSystem::TriggerEvent(EVENT_NETWORK_SHUTDOWN);

	int error = WSACleanup();
	if(error == SOCKET_ERROR)
	{
		NetworkUtils::ReportError();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BNetworkSystem::OnUpdate(NamedProperties &)
{
	BEventSystem::TriggerEvent(BNetworkSystem::EVENT_NETWORK_UPDATE);
}
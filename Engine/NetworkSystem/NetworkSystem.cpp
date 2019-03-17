#include "Engine/NetworkSystem/NetworkSystem.hpp"

#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC size_t NetworkSystem::GAME_PORT = 4334;
STATIC char const * NetworkSystem::EVENT_NETWORK_STARTUP = " NetworkStartup";
STATIC char const * NetworkSystem::EVENT_NETWORK_SHUTDOWN = "NetworkShutdown";
STATIC char const * NetworkSystem::EVENT_NETWORK_UPDATE = "NetworkUpdateEvent";


//-------------------------------------------------------------------------------------------------
STATIC bool NetworkSystem::Startup()
{
	WSADATA wsa_data;
	//version 2.2
	WORD version = MAKEWORD(2, 2);
	int error = WSAStartup(version, &wsa_data);
	if(error == SOCKET_ERROR)
	{
		NetworkUtils::ReportError();
	}

	BEventSystem::RegisterEvent(EVENT_ENGINE_UPDATE, &NetworkSystem::OnUpdate);
	BEventSystem::TriggerEvent(EVENT_NETWORK_STARTUP);

	return true;
}


//-------------------------------------------------------------------------------------------------
STATIC void NetworkSystem::Shutdown()
{
	BEventSystem::TriggerEvent(EVENT_NETWORK_SHUTDOWN);

	int error = WSACleanup();
	if(error == SOCKET_ERROR)
	{
		NetworkUtils::ReportError();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void NetworkSystem::OnUpdate(NamedProperties &)
{
	BEventSystem::TriggerEvent(NetworkSystem::EVENT_NETWORK_UPDATE);
}
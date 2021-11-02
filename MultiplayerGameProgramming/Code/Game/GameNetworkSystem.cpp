#include "Game/GameNetworkSystem.hpp"

#include <vector>
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/NetworkSystem/Sockets/UDPSocket.hpp"
#include "Engine/NetworkSystem/Sockets/SocketAddress.hpp"


//-------------------------------------------------------------------------------------------------
GameNetworkSystem::GameNetworkSystem()
{
	BConsoleSystem* consoleSystem = BConsoleSystem::CreateOrGetSystem();
	if (consoleSystem)
	{
		consoleSystem->Register("test", this, &GameNetworkSystem::Test, " : Test network system functionality");
	}

	size_t port = PORT_START;
	m_listeningSocket = NetworkUtils::CreateUDPSocket(eSocketAddressFamily_IPv4);
	if (m_listeningSocket == nullptr)
	{
		BConsoleSystem::AddLog("GameNetworkSystem - Error creating socket", BConsoleSystem::BAD);
		return;
	}

	while (port < PORT_START + PORT_RETRY_RANGE)
	{
		m_ipaddress = NetworkUtils::CreateIPv4FromString("localhost", port);
		if (m_listeningSocket->Bind(m_ipaddress) == NO_ERROR)
		{
			std::string success = Stringf("GameNetworkSystem - Connected to [%s]", m_ipaddress->GetAddress());
			BConsoleSystem::AddLog(success, BConsoleSystem::INFO);
			return;
		}

		port += 1;
	}

	BConsoleSystem::AddLog("GameNetworkSystem - Error binding socket", BConsoleSystem::BAD);
	return;
}

//-------------------------------------------------------------------------------------------------
GameNetworkSystem::~GameNetworkSystem()
{

}

//-------------------------------------------------------------------------------------------------
void GameNetworkSystem::Update()
{
	const int PACKET_MAX_SIZE = 1300;

	char buffer[PACKET_MAX_SIZE];
	SocketAddressPtr dummy;
	int byteCountReceived = m_listeningSocket->ReceiveFrom((void *)buffer, PACKET_MAX_SIZE - 1, dummy);
	if (byteCountReceived >= 0)
	{
		byteCountReceived = Min(byteCountReceived, PACKET_MAX_SIZE - 1);
		buffer[byteCountReceived] = '\0';
		std::string success = Stringf("Received %d Bytes. Message = [%s]", byteCountReceived, buffer);
		BConsoleSystem::AddLog(success, BConsoleSystem::GOOD);
	}
}


//-------------------------------------------------------------------------------------------------
void GameNetworkSystem::Test(const Command& command)
{
	if (m_ipaddress == nullptr || m_listeningSocket == nullptr)
	{
		BConsoleSystem::AddLog("ipaddress or updsocket not created correctly.", BConsoleSystem::BAD);
		return;
	}

	std::string message = command.GetArg(0, "");

	int byteCountSent = m_listeningSocket->SendTo((const void*)message.c_str(), message.length() + 1, m_ipaddress);

	if (byteCountSent >= 0)
	{
		std::string success = Stringf("Sent %d Bytes. Message = [%s]", byteCountSent, message.c_str());
		BConsoleSystem::AddLog(success, BConsoleSystem::GOOD);
	}
	else
	{
		std::string failure = Stringf("Failed to send. Message = [%s]", message.c_str());
		BConsoleSystem::AddLog(failure, BConsoleSystem::BAD);
	}
}

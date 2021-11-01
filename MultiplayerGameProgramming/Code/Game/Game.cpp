#include "Game/Game.hpp"

#include <vector>
#include "Engine/Utils/StreamReader.hpp"
#include "Engine/Utils/StreamWriter.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/NetworkSystem/Sockets/UDPSocket.hpp"


//-------------------------------------------------------------------------------------------------
Game * g_GameSystem = nullptr;


//-------------------------------------------------------------------------------------------------
void UDPSendCommand(const Command& command)
{
	if (g_GameSystem->m_ipaddress == nullptr || g_GameSystem->m_udpsocket == nullptr)
	{
		BConsoleSystem::AddLog("ipaddress or updsocket not created correctly.", BConsoleSystem::BAD);
	}

	std::string message = command.GetArg(0, "");

	int byteCountSent = g_GameSystem->m_udpsocket->SendTo((const void*)message.c_str(), message.length() * 4 + 1, g_GameSystem->m_ipaddress);

	if (byteCountSent >= 0)
	{
		std::string success = Stringf("Sent %d Bytes. Message = %s", byteCountSent, message.c_str());
		BConsoleSystem::AddLog(success, BConsoleSystem::GOOD);
	}
	else
	{
		std::string failure = Stringf("Failed to send. Message = ", message.c_str());
		BConsoleSystem::AddLog(failure, BConsoleSystem::BAD);
	}
}


//-------------------------------------------------------------------------------------------------
Game::Game()
{
	BConsoleSystem::Register("udp_send", &UDPSendCommand, " [msg] : Send message with udp sockets.");

	m_ipaddress = NetworkUtils::CreateIPv4FromString("localhost");
	m_udpsocket = NetworkUtils::CreateUDPSocket(eSocketAddressFamily_IPv4);
	m_udpsocket->Bind(m_ipaddress);
	m_udpsocket->SetBlocking(false);
}


//-------------------------------------------------------------------------------------------------
Game::~Game()
{
	
}


//-------------------------------------------------------------------------------------------------
void Game::Update()
{
	char buffer[255];
	SocketAddressPtr dummy;
	int byteCountReceived = m_udpsocket->ReceiveFrom((void *)buffer, 255, dummy);
	if (byteCountReceived > 0)
	{
		buffer[254] = '\0';
		std::string success = Stringf("Received %d Bytes. Message = %s", byteCountReceived, buffer);
		BConsoleSystem::AddLog(success, BConsoleSystem::GOOD);
	}
}


//-------------------------------------------------------------------------------------------------
void Game::Render() const
{

}
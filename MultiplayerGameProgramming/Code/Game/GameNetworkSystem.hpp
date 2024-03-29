#pragma once

#include "Engine/Utils/NetworkUtils.hpp"
#include "Engine/DebugSystem/Command.hpp"


//-------------------------------------------------------------------------------------------------
class GameNetworkSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	SocketAddressPtr m_ipaddress = nullptr;
	UDPSocketPtr m_listeningSocket = nullptr;
	const size_t PORT_START = 4325;
	const size_t PORT_RETRY_RANGE = 10;
	//const int PACKET_MAX_SIZE = 1300;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	GameNetworkSystem();
	~GameNetworkSystem();
	void Update();

	void Test(const Command& command);
};
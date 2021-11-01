#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class SocketAddress
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	friend class TCPSocket;
	friend class UDPSocket;
	sockaddr m_sockAddr;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	SocketAddress(uint32_t address, uint16_t port);
	SocketAddress(const sockaddr& sockAddr);

	size_t GetSize() const;
	const char* GetAddress();

private:
	sockaddr_in* GetAsSockAddrIn();
};
typedef std::shared_ptr<SocketAddress> SocketAddressPtr;
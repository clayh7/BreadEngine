#include "Engine/NetworkSystem/Sockets/UDPSocket.hpp"

#include "Engine/NetworkSystem/Sockets/SocketAddress.hpp"
#include "Engine/NetworkSystem/BNetworkSystem.hpp"


//-------------------------------------------------------------------------------------------------
UDPSocket::~UDPSocket()
{
	closesocket(m_socket);
}


//-------------------------------------------------------------------------------------------------
int UDPSocket::Bind(SocketAddressPtr address)
{
	int error = bind(m_socket, &(address->m_sockAddr), address->GetSize());
	if(error == SOCKET_ERROR)
	{
		return NetworkUtils::ReportError();
	}
	m_address = address;
	return NO_ERROR;
}


//-------------------------------------------------------------------------------------------------
int UDPSocket::SendTo(const void* data, int length, SocketAddressPtr toAddress)
{
	int byteSentCount = sendto(m_socket, static_cast<const char*>(data), length, 0, &(toAddress->m_sockAddr), toAddress->GetSize());
	if(byteSentCount >= 0)
	{
		return byteSentCount;
	}
	else
	{
		return NetworkUtils::ReportError();
	}
}


//-------------------------------------------------------------------------------------------------
int UDPSocket::ReceiveFrom(void* buffer, int length, SocketAddressPtr out_fromAddress)
{
	sockaddr fromAddress;
	int fromLength = sizeof(fromAddress);
	int readByteCount = recvfrom(m_socket, static_cast<char*>(buffer), length, 0, &fromAddress, &fromLength);
	out_fromAddress = std::make_shared<SocketAddress>(fromAddress);
	if(readByteCount >= 0)
	{
		return readByteCount;
	}
	else
	{
		// WOULD_BLOCK_ERROR is expected if we're not blocking
		int errorCode = WSAGetLastError();
		if (errorCode != NetworkUtils::WOULD_BLOCK_ERROR)
		{
			NetworkUtils::ReportError();
		}
		return readByteCount;
	}
}


//-------------------------------------------------------------------------------------------------
void UDPSocket::SetBlocking(bool isBlocking)
{
	u_long blocking = isBlocking ? 0 : 1;
	int error = ioctlsocket(m_socket, FIONBIO, &blocking);
	if(error == SOCKET_ERROR)
	{
		NetworkUtils::ReportError();
	}
}


//-------------------------------------------------------------------------------------------------
UDPSocket::UDPSocket(SOCKET inSocket)
	: m_socket(inSocket)
	, m_address(nullptr)
{
	SetBlocking(false);
}

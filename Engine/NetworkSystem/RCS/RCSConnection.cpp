#include "Engine/NetworkSystem/RCS/RCSConnection.hpp"

#include "Engine/NetworkSystem/Sockets/TCPSocket.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/NetworkSystem/RCS/RemoteCommandServer.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"


//-------------------------------------------------------------------------------------------------
RCSConnection::RCSConnection(TCPSocketPtr tcpSocket)
	: m_tcpSocket(tcpSocket)
	, m_messageBufferIndex(0)
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
RCSConnection::~RCSConnection()
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void RCSConnection::Send(eRCSMessageType messageType, char const * message) const
{
	if(m_tcpSocket)
	{
		m_tcpSocket->Send((void*)&messageType, 1);
		m_tcpSocket->Send((void*)message, strlen(message));
		char end = eRCSMessageType_END;
		m_tcpSocket->Send((void*)&end, 1);
	}
}


//-------------------------------------------------------------------------------------------------
void RCSConnection::Receive()
{
	if(!m_tcpSocket)
	{
		return;
	}

	char buffer[BUFFER_SIZE];
	int readLength = m_tcpSocket->Receive(buffer, BUFFER_SIZE);

	while(readLength > 0)
	{
		for(int index = 0; index < readLength; ++index)
		{
			char readChar = buffer[index];
			AddToBuffer(readChar);
			if(readChar == eRCSMessageType_END)
			{
				CreateMessageEvent();
				ClearMessageBuffer();
			}
		}
		readLength = m_tcpSocket->Receive(buffer, BUFFER_SIZE);
	}
}


//-------------------------------------------------------------------------------------------------
SOCKET RCSConnection::GetSocket() const
{
	return m_tcpSocket->m_socket;
}


//-------------------------------------------------------------------------------------------------
std::string RCSConnection::GetAddress() const
{
	return m_tcpSocket->GetAddressString();
}


//-------------------------------------------------------------------------------------------------
void RCSConnection::AddToBuffer(char readChar)
{
	m_messageBuffer[m_messageBufferIndex] = readChar;
	m_messageBufferIndex += 1;
}


//-------------------------------------------------------------------------------------------------
void RCSConnection::ClearMessageBuffer()
{
	memset(m_messageBuffer, 0, RCSConnection::BUFFER_SIZE);
	m_messageBufferIndex = 0;
}


//-------------------------------------------------------------------------------------------------
void RCSConnection::CreateMessageEvent()
{
	NamedProperties data;
	data.Set("MessageType", (eRCSMessageType)m_messageBuffer[0]);
	data.Set("Message", &m_messageBuffer[1]);
	BEventSystem::TriggerEvent(RemoteCommandServer::EVENT_RCS_MESSAGE, data);
}
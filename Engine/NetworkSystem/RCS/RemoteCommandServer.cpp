#include "Engine/NetworkSystem/RCS/RemoteCommandServer.hpp"

#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/NetworkSystem/BNetworkSystem.hpp"
#include "Engine/NetworkSystem/RCS/RCSConnection.hpp"
#include "Engine/NetworkSystem/Sockets/TCPSocket.hpp"
#include "Engine/NetworkSystem/Sockets/SocketAddress.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const int RemoteCommandServer::DEFAULT_WAIT = 0;
STATIC size_t RemoteCommandServer::RCS_PORT = 4325;
STATIC char const * RemoteCommandServer::EVENT_RCS_MESSAGE = "RCSMessageEvent";
STATIC RemoteCommandServer * RemoteCommandServer::s_Instance = nullptr;


//-------------------------------------------------------------------------------------------------
void RCSHost(Command const &)
{
	RemoteCommandServer const * rcs = RemoteCommandServer::CreateOrGetInstance();
	if(rcs->IsHost())
	{
		BConsoleSystem::AddLog("Already hosting.", BConsoleSystem::BAD);
		return;
	}

	if(rcs->IsClient())
	{
		BConsoleSystem::AddLog("Clients can not also host.", BConsoleSystem::BAD);
		return;
	}

	if(RemoteCommandServer::Host())
	{
		std::string success = Stringf("Host successful. Listening on %s", rcs->GetListenAddress().c_str());
		BConsoleSystem::AddLog(success, BConsoleSystem::GOOD);
	}
	else
	{
		BConsoleSystem::AddLog("Failed to host.", BConsoleSystem::BAD);
	}
}


//-------------------------------------------------------------------------------------------------
void RCSJoin(Command const & command)
{
	RemoteCommandServer const * rcs = RemoteCommandServer::CreateOrGetInstance();
	if(rcs->IsConnected())
	{
		BConsoleSystem::AddLog("Already connected to host.", BConsoleSystem::BAD);
		return;
	}

	//Parse host address string
	std::string address = "";
	if(!command.HasArg(0))
	{
		address = NetworkUtils::GetLocalHostName();
	}
	else
	{
		address = command.GetArg(0, "error");
	}

	if(RemoteCommandServer::Join(address.c_str()))
	{
		std::string success = Stringf("Joined server: %s", address.c_str());
		BConsoleSystem::AddLog(success, BConsoleSystem::GOOD);
	}
	else
	{
		BConsoleSystem::AddLog("Failed to join server.", BConsoleSystem::BAD);
	}
}


//-------------------------------------------------------------------------------------------------
void RCSLeave(Command const &)
{
	RemoteCommandServer const * rcs = RemoteCommandServer::CreateOrGetInstance();
	if(!rcs->IsConnected())
	{
		BConsoleSystem::AddLog("Nothing to leave.", BConsoleSystem::BAD);
		return;
	}

	if(rcs->IsHost())
	{
		RemoteCommandServer::Leave();
		BConsoleSystem::AddLog("Stopped hosting.", BConsoleSystem::GOOD);
	}

	if(rcs->IsClient())
	{
		RemoteCommandServer::Leave();
		BConsoleSystem::AddLog("Disconnected from host.", BConsoleSystem::GOOD);
	}
}


//-------------------------------------------------------------------------------------------------
void RCSInfo(Command const &)
{
	RemoteCommandServer const * rcs = RemoteCommandServer::CreateOrGetInstance();
	if(rcs->IsHost())
	{
		BConsoleSystem::AddLog("Status: Host", BConsoleSystem::INFO);
		std::string hostString = Stringf("Host: %s", rcs->GetListenAddress().c_str());
		BConsoleSystem::AddLog(hostString, BConsoleSystem::GOOD);

		std::vector<std::string> connectionAddressList = rcs->GetConnectionAddresses();
		BConsoleSystem::AddLog(Stringf("Connections: %u", connectionAddressList.size()), BConsoleSystem::GOOD);
		for(size_t clientIndex = 0; clientIndex < connectionAddressList.size(); ++clientIndex)
		{
			std::string clientString = Stringf("[%u] %s", clientIndex, connectionAddressList[clientIndex].c_str());
			BConsoleSystem::AddLog(clientString, BConsoleSystem::GOOD);
		}
	}
	else if(rcs->IsClient())
	{
		BConsoleSystem::AddLog("Status: Client", BConsoleSystem::INFO);
		std::vector<std::string> connectionAddressList = rcs->GetConnectionAddresses();
		std::string clientString = Stringf("Client: %s", connectionAddressList[0].c_str());
		BConsoleSystem::AddLog(clientString, BConsoleSystem::GOOD);
	}
	else
	{
		BConsoleSystem::AddLog("Status: Disconnected", BConsoleSystem::INFO);
	}
}


//-------------------------------------------------------------------------------------------------
void RCSSend(Command const & command)
{
	RemoteCommandServer const * rcs = RemoteCommandServer::CreateOrGetInstance();
	if(!rcs->IsConnected())
	{
		BConsoleSystem::AddLog("Not connected.", BConsoleSystem::BAD);
		return;
	}

	RemoteCommandServer::Send(eRCSMessageType_COMMAND, command.GetRemainingString(0));
}


//-------------------------------------------------------------------------------------------------
STATIC void RemoteCommandServer::Startup()
{
	if(!s_Instance)
	{
		s_Instance = new RemoteCommandServer();
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::Shutdown()
{
	if(s_Instance)
	{
		delete s_Instance;
		s_Instance = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC RemoteCommandServer * RemoteCommandServer::CreateOrGetInstance()
{
	if(!s_Instance)
	{
		Startup();
	}

	return s_Instance;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Host(uint32_t port /*= RCS_PORT*/)
{
	if(!s_Instance)
	{
		//Startup required
		return false;
	}

	if(!s_Instance->IsConnected())
	{
		char const * host = NetworkUtils::GetLocalHostName();
		SocketAddressPtr address = NetworkUtils::CreateIPv4FromString(host, port);
		s_Instance->StartTCPListener(address);
		return s_Instance->IsHost();
	}

	//Already host or client
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Join(char const * host, uint32_t port /*= RCS_PORT*/)
{
	if(!s_Instance)
	{
		//Startup required
		return false;
	}

	if(!s_Instance->IsConnected())
	{
		if(s_Instance->m_connections.size() > 0)
		{
			return false;
		}
		SocketAddressPtr address = NetworkUtils::CreateIPv4FromString(host, port);
		s_Instance->CreateRCSConnection(address);
		return s_Instance->IsClient();
	}

	//Already host or client
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Send(eRCSMessageType const & type, std::string const & message)
{
	if(!s_Instance)
	{
		//Startup required
		return false;
	}

	if(s_Instance->IsConnected())
	{
		s_Instance->SendRCSMessage(type, message);
		return true;
	}

	//Not connected
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Leave()
{
	if(!s_Instance)
	{
		//Startup required
		return false;
	}

	if(s_Instance->IsConnected())
	{
		s_Instance->Disconnect();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
RemoteCommandServer::RemoteCommandServer()
	: m_state(eRCSState_DISCONNECTED)
{
	BEventSystem::RegisterEvent(BNetworkSystem::EVENT_NETWORK_UPDATE, this, &RemoteCommandServer::OnUpdate);
	BEventSystem::RegisterEvent(EVENT_RCS_MESSAGE, this, &RemoteCommandServer::OnMessage);

	BConsoleSystem::Register("rcs_host", RCSHost, ": Host a local server.");
	BConsoleSystem::Register("rcs_join", RCSJoin, " [host address] : Join server.");
	BConsoleSystem::Register("rcs_leave", RCSLeave, ": Leave current server.");
	BConsoleSystem::Register("rcs_info", RCSInfo, ": Prints current network state.");
	BConsoleSystem::Register("rcs_send", RCSSend, " [cmd] : Sends command to all connections.");
}


//-------------------------------------------------------------------------------------------------
RemoteCommandServer::~RemoteCommandServer()
{
	for(RCSConnection * conn : m_connections)
	{
		delete conn;
		conn = nullptr;
	}
	m_connections.clear();
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::OnUpdate(NamedProperties &)
{
	if(!IsConnected())
	{
		return;
	}

	AcceptNewConnections();
	UpdateConnections();
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::OnMessage(NamedProperties & params)
{
	eRCSMessageType messageType;
	params.Get("MessageType", messageType);
	std::string message;
	params.Get("Message", message);

	if(messageType == eRCSMessageType_COMMAND)
	{
		if(BConsoleSystem::s_System)
		{
			BConsoleSystem::s_System->RunCommand(message, true);
		}
	}
	else if(messageType == eRCSMessageType_ECHO)
	{
		BConsoleSystem::AddLog(Stringf("REMOTE: %s", message.c_str()), BConsoleSystem::REMOTE, true);
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::StartTCPListener(SocketAddressPtr address)
{
	m_listener = NetworkUtils::CreateTCPSocket(eSocketAddressFamily_IPv4);
	if(!m_listener)
	{
		return;
	}
	int error = m_listener->Bind(address);
	if(error != NO_ERROR)
	{
		return;
	}
	error = m_listener->Listen();
	if(error == NO_ERROR)
	{
		m_state = eRCSState_HOST;
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::CreateRCSConnection(SocketAddressPtr address)
{
	TCPSocketPtr tcpSocket = NetworkUtils::CreateTCPSocket(eSocketAddressFamily_IPv4);
	if(tcpSocket)
	{
		//Must block before connect
		tcpSocket->SetBlocking(true);
		int error = tcpSocket->Connect(address);
		if(error == NO_ERROR)
		{
			CreateRCSConnection(tcpSocket);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::CreateRCSConnection(TCPSocketPtr tcpSocket)
{
	if(tcpSocket)
	{
		tcpSocket->SetBlocking(false);
		RCSConnection * connection = new RCSConnection(tcpSocket);
		m_connections.push_back(connection);
	}

	if(!IsConnected())
	{
		m_state = eRCSState_CLIENT;
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::SendRCSMessage(eRCSMessageType const & type, std::string const & message)
{
	for(RCSConnection const * conn : m_connections)
	{
		if(conn)
		{
			conn->Send(type, message.c_str());
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::Disconnect()
{
	if(IsHost())
	{
		m_listener = nullptr;
		for(RCSConnection * conn : m_connections)
		{
			if(conn)
			{
				delete conn;
				conn = nullptr;
			}
		}
		m_connections.clear();
		m_state = eRCSState_DISCONNECTED;
	}

	if(IsClient())
	{
		ASSERT_RECOVERABLE(m_connections.size() == 1, "Incorrect number of client connections.")
			delete m_connections[0];
		m_connections[0] = nullptr;
		m_connections.clear();
		m_state = eRCSState_DISCONNECTED;
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::AcceptNewConnections()
{
	if(!IsHost())
	{
		return;
	}

	if(m_listener)
	{
		TCPSocketPtr tcpSocket = m_listener->Accept();
		if(tcpSocket)
		{
			CreateRCSConnection(tcpSocket);
			BConsoleSystem::AddLog("Client connected: " + tcpSocket->GetAddressString(), BConsoleSystem::REMOTE);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::UpdateConnections()
{
	std::vector<WSAPOLLFD> fdArray;
	FDArrayFill(fdArray);
	if(fdArray.size() <= 0)
	{
		return;
	}

	uint32_t result = WSAPoll(&fdArray[0], fdArray.size(), DEFAULT_WAIT);
	if(result <= 0)
	{
		return;
	}

	FDArrayProcess(fdArray);
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::FDArrayFill(std::vector<WSAPOLLFD> & fdArray)
{
	for(size_t connIndex = 0; connIndex < m_connections.size(); ++connIndex)
	{
		RCSConnection * conn = m_connections[connIndex];
		if(conn)
		{
			WSAPOLLFD poll;
			poll.fd = conn->GetSocket();
			poll.events = POLLRDNORM | POLLWRNORM;
			fdArray.push_back(poll);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::FDArrayProcess(std::vector<WSAPOLLFD> & fdArray)
{
	for(size_t fdIndex = 0; fdIndex < fdArray.size(); )
	{
		if(!m_connections[fdIndex])
		{
			continue;
		}

		//Connection can be read from
		if(fdArray[fdIndex].revents & POLLRDNORM)
		{
			m_connections[fdIndex]->Receive();
		}

		//Connection can be sent to
		if(fdArray[fdIndex].revents & POLLWRNORM)
		{
			//Nothing
		}

		//Connection left
		if(fdArray[fdIndex].revents & POLLHUP)
		{
			HandleDisconnect(m_connections[fdIndex]);

			//Remove from fdArray
			std::vector<WSAPOLLFD>::iterator fdIter = fdArray.begin() + fdIndex;
			fdArray.erase(fdIter);

			//Remove from 
			std::vector<RCSConnection*>::iterator connIter = m_connections.begin() + fdIndex;
			m_connections.erase(connIter);
		}
		else
		{
			++fdIndex;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::HandleDisconnect(RCSConnection *& conn)
{
	if(!conn)
	{
		return;
	}

	if(IsHost())
	{
		BConsoleSystem::AddLog(Stringf("Client left: %s", conn->GetAddress().c_str()), BConsoleSystem::REMOTE);
	}

	if(IsClient())
	{
		BConsoleSystem::AddLog(Stringf("Host disconnected."), BConsoleSystem::REMOTE);
		m_state = eRCSState_DISCONNECTED;
	}

	delete conn;
	conn = nullptr;
}


//-------------------------------------------------------------------------------------------------
bool RemoteCommandServer::IsConnected() const
{
	return m_state != eRCSState_DISCONNECTED;
}


//-------------------------------------------------------------------------------------------------
bool RemoteCommandServer::IsHost() const
{
	return m_state == eRCSState_HOST;
}


//-------------------------------------------------------------------------------------------------
bool RemoteCommandServer::IsClient() const
{
	return m_state == eRCSState_CLIENT;
}


//-------------------------------------------------------------------------------------------------
std::string RemoteCommandServer::GetListenAddress() const
{
	if(m_listener)
	{
		return m_listener->GetAddressString();
	}
	else
	{
		return "error";
	}
}


//-------------------------------------------------------------------------------------------------
std::vector<std::string> RemoteCommandServer::GetConnectionAddresses() const
{
	std::vector<std::string> connectionAddressList;

	for(RCSConnection * conn : m_connections)
	{
		if(conn)
		{
			connectionAddressList.push_back(conn->GetAddress());
		}
	}

	return connectionAddressList;
}

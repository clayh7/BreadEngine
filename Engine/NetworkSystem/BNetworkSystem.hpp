#pragma once


//-------------------------------------------------------------------------------------------------
class NamedProperties;


//-------------------------------------------------------------------------------------------------
class BNetworkSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static size_t GAME_PORT;
	static const char * EVENT_NETWORK_STARTUP;
	static const char * EVENT_NETWORK_SHUTDOWN;
	static const char * EVENT_NETWORK_UPDATE;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static bool Startup();
	static void Shutdown();

private:
	static void OnUpdate(NamedProperties &);
};
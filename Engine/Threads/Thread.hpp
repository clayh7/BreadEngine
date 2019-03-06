#pragma once
#include <thread>


//-------------------------------------------------------------------------------------------------
typedef void ( EntryCallback )( void * );


//-------------------------------------------------------------------------------------------------
class Thread
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	std::thread m_handle;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Thread( EntryCallback * functionPtr );
	void Join( );
	void Detach( );
};
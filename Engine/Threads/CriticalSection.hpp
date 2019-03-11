#pragma once
#include <mutex>


//-------------------------------------------------------------------------------------------------
class CriticalSection
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	std::mutex m_mutex;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	CriticalSection();
	CriticalSection(CriticalSection const & copy) = delete; // removes the copy constructor

	void Lock();
	void Unlock();
	bool TryLock();
};
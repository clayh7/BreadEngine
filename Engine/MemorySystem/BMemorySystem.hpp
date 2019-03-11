#pragma once

#include <map>
#include <vector>
#include "Engine/MemorySystem/Callstack.hpp"
#include "Engine/MemorySystem/UntrackedAllocator.hpp"
#include "Engine/Threads/CriticalSection.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class CallstackStats
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	size_t m_totalAllocations;
	size_t m_totalBytes;
	char const * m_lineAndNumber;
	Callstack * m_callstackPtr;
};


//-------------------------------------------------------------------------------------------------
// #TODO: Maybe actually go through and tag shit?
enum eMemoryTag
{
	eMemoryTag_UNTRACKED,
	eMemoryTag_DEFAULT,
	eMemoryTag_UPDATE_ENGINE,
	eMemoryTag_UPDATE_GAME,
	eMemoryTag_RENDER_ENGINE,
	eMemoryTag_RENDER_GAME,
};


//-------------------------------------------------------------------------------------------------
typedef std::map<void*, Callstack*, std::less<void*>, UntrackedAllocator<std::pair<void*, Callstack*>>> UntrackedCallstackMap;
typedef std::map<uint32_t, CallstackStats, std::less<uint32_t>, UntrackedAllocator<std::pair<uint32_t, CallstackStats>>> UntrackedCallstackStatsMap;


//-------------------------------------------------------------------------------------------------
class BMemorySystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
private:
	static BMemorySystem * s_MemorySystem;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	UntrackedCallstackMap m_callstackMap;
	UntrackedCallstackStatsMap m_callstackStatsMap;
	CriticalSection m_criticalSectionCallstackMap;

private:
	size_t m_startupAllocations;
	size_t m_numAllocations;
	size_t m_totalAllocated;
	size_t m_highestTotalAllocated;

	float m_timeStampOfPreviousAnalysis;
	size_t m_allocationsForOneSecond;
	size_t m_allocationsInTheLastSecond;
	float m_averageAllocationsPerSecond;
	size_t m_deallocationsForOneSecond;
	size_t m_deallocationsInTheLastSecond;
	float m_averageDeallocationsPerSecond;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void Update();
	static void Flush();
	static BMemorySystem * GetOrCreateSystem();
	static void GetMemoryAllocationsString(std::string & allocationString);
	static void GetMemoryAveragesString(std::string & averageString);

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	void * Allocate(size_t numBytes, eMemoryTag tag);
	void Deallocate(void * ptr);

private:
	BMemorySystem();
	~BMemorySystem();

	void SystemUpdate();
	void SystemGetMemoryAllocationString(std::string & allocationString);
	void SystemGetMemoryAveragesString(std::string & averageString);
	void SystemFlush();
	void PopulateCallstackStats();
	void CleanUpCallstackStats();
	void LockCallstackMap();
	void UnlockCallstackMap();
};

extern void * operator new(size_t numBytes);
extern void * operator new (size_t numBytes, eMemoryTag tag);
extern void * operator new[](size_t numBytes);
extern void * operator new[](size_t numBytes, eMemoryTag tag);
extern void operator delete (void * ptr);
extern void operator delete (void * ptr, eMemoryTag);
extern void operator delete[](void * ptr);
extern void operator delete[](void * ptr, eMemoryTag);
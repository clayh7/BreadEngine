#include "Engine/MemorySystem/BMemorySystem.hpp"

#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC BMemorySystem * BMemorySystem::s_MemorySystem = nullptr;
bool g_SkipTracking = false;


//-------------------------------------------------------------------------------------------------
size_t * CreateMemoryBlock(size_t numBytes, eMemoryTag tag)
{
	BProfiler::IncrementNews();

	// This is the memory layout, we need to leave 8 bytes beforehand free
	// [(4)NumBytes][(4)MemoryTag][(NumBytes)MemoryBlock]
	size_t totalSize = numBytes; // Memory Block
	totalSize += sizeof(size_t); // Memory Tag
	totalSize += sizeof(size_t); // Number of Bytes
	size_t * ptr = (size_t*)malloc(totalSize);
	ptr[0] = numBytes; // Number of Bytes
	ptr[1] = tag; // Memory Tag
	return &ptr[2]; // Move the pointer back to the beginning of the Memory Block
}


//-------------------------------------------------------------------------------------------------
void DestroyMemoryBlock(void * ptr, size_t & numBytes, eMemoryTag & tag)
{
	BProfiler::IncrementDeletes();

	size_t * ptrStart = (size_t *)ptr; //Memory Block
	ptrStart -= 1; // Memory Tag
	ptrStart -= 1; // Number of Bytes
	numBytes = ptrStart[0]; // Number of Bytes
	tag = (eMemoryTag)ptrStart[1]; // Memory Tag
	free(ptrStart);
}


//-------------------------------------------------------------------------------------------------
// new / delete Overrides
//-------------------------------------------------------------------------------------------------
#if MEMORY_TRACKING >= 1
//-------------------------------------------------------------------------------------------------
void * operator new (size_t numBytes)
{
	if(g_SkipTracking)
	{
		return CreateMemoryBlock(numBytes, eMemoryTag_DEFAULT);
	}

	BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
	return MSystem->Allocate(numBytes, eMemoryTag_DEFAULT);
}


//-------------------------------------------------------------------------------------------------
void * operator new (size_t numBytes, eMemoryTag tag)
{
	if(tag == eMemoryTag_UNTRACKED)
	{
		return CreateMemoryBlock(numBytes, tag);
	}

	BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
	return MSystem->Allocate(numBytes, tag);
}


//-------------------------------------------------------------------------------------------------
void * operator new [](size_t numBytes)
{
	BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
	return MSystem->Allocate(numBytes, eMemoryTag_DEFAULT);
}


//-------------------------------------------------------------------------------------------------
void * operator new [](size_t numBytes, eMemoryTag tag)
{
	if(tag == eMemoryTag_UNTRACKED)
	{
		return CreateMemoryBlock(numBytes, tag);
	}

	BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
	return MSystem->Allocate(numBytes, tag);
}


//-------------------------------------------------------------------------------------------------
void operator delete (void * ptr)
{
	if(g_SkipTracking)
	{
		size_t unused1;
		eMemoryTag unused2;
		DestroyMemoryBlock(ptr, unused1, unused2);
	}
	else
	{
		BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
		MSystem->Deallocate(ptr);
	}
}


//-------------------------------------------------------------------------------------------------
void operator delete (void * ptr, eMemoryTag tag)
{
	if(tag == eMemoryTag_UNTRACKED)
	{
		size_t unused1;
		eMemoryTag unused2;
		DestroyMemoryBlock(ptr, unused1, unused2);
	}
	else
	{
		BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
		MSystem->Deallocate(ptr);
	}
}


//-------------------------------------------------------------------------------------------------
void operator delete [](void * ptr)
{
	BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
	MSystem->Deallocate(ptr);
}


//-------------------------------------------------------------------------------------------------
void operator delete [](void * ptr, eMemoryTag tag)
{
	if(tag == eMemoryTag_UNTRACKED)
	{
		size_t unused1;
		eMemoryTag unused2;
		DestroyMemoryBlock(ptr, unused1, unused2);
	}
	else
	{
		BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
		MSystem->Deallocate(ptr);
	}
}
#endif // MEMORY_TRACKING >= 1


//-------------------------------------------------------------------------------------------------
STATIC void BMemorySystem::Startup()
{
	if(!s_MemorySystem)
	{
		CallstackSystem::Startup();
		g_SkipTracking = true;
		s_MemorySystem = new BMemorySystem();
		g_SkipTracking = false;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMemorySystem::Shutdown()
{
	if(s_MemorySystem)
	{
		delete s_MemorySystem;
		s_MemorySystem = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMemorySystem::Update()
{
	if(s_MemorySystem)
	{
		s_MemorySystem->SystemUpdate();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMemorySystem::Flush()
{
	if(s_MemorySystem)
	{
		s_MemorySystem->SystemFlush();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BMemorySystem * BMemorySystem::GetOrCreateSystem()
{
	if(!s_MemorySystem)
	{
		Startup();
	}
	return s_MemorySystem;
}


//-------------------------------------------------------------------------------------------------
STATIC void BMemorySystem::GetMemoryAllocationsString(std::string & allocationString)
{
	if(s_MemorySystem)
	{
		s_MemorySystem->SystemGetMemoryAllocationString(allocationString);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMemorySystem::GetMemoryAveragesString(std::string & averageString)
{
	if(s_MemorySystem)
	{
		s_MemorySystem->SystemGetMemoryAveragesString(averageString);
	}
}


//-------------------------------------------------------------------------------------------------
void * BMemorySystem::Allocate(size_t numBytes, eMemoryTag tag)
{
	size_t * ptr = CreateMemoryBlock(numBytes, tag);

	//Make room for saving the meta data
	m_numAllocations += 1;
	m_allocationsForOneSecond += 1;
	m_totalAllocated += numBytes;

	//Track high-water mark
	if(m_totalAllocated > m_highestTotalAllocated)
	{
		m_highestTotalAllocated = m_totalAllocated;
	}

	//Add allocation stack to map
	LockCallstackMap();
	auto foundAllocation = m_callstackMap.find(ptr);
	if(foundAllocation == m_callstackMap.end()) // No duplicates
	{
		Callstack * currentCallstack = CallstackSystem::Allocate(2);
		m_callstackMap.insert(std::pair<void*, Callstack*>(ptr, currentCallstack));
	}
	else
	{
		__debugbreak();
	}
	UnlockCallstackMap();

	return ptr;
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::Deallocate(void * ptr)
{
	size_t numBytes;
	eMemoryTag tag;
	DestroyMemoryBlock(ptr, numBytes, tag);

	//Update allocation trackers
	if(m_numAllocations == 0)
	{
		//Too many deallocations
		__debugbreak();
	}
	else
	{
		m_numAllocations -= 1;
		m_deallocationsForOneSecond += 1;
		m_totalAllocated -= numBytes;
	}

	//Remove allocation from map
	LockCallstackMap();
	auto foundAllocation = m_callstackMap.find(ptr);
	if(foundAllocation != m_callstackMap.end())
	{
		CallstackSystem::Free(foundAllocation->second);
		m_callstackMap.erase(foundAllocation);
	}
	else
	{
		__debugbreak();
	}
	UnlockCallstackMap();
}


//-------------------------------------------------------------------------------------------------
BMemorySystem::BMemorySystem()
	: m_startupAllocations(0U)
	, m_numAllocations(0)
	, m_totalAllocated(0)
	, m_highestTotalAllocated(0)
	, m_timeStampOfPreviousAnalysis(0.f)
	, m_allocationsForOneSecond(0)
	, m_allocationsInTheLastSecond(0)
	, m_averageAllocationsPerSecond(0.f)
	, m_deallocationsForOneSecond(0)
	, m_deallocationsInTheLastSecond(0)
	, m_averageDeallocationsPerSecond(0.f)
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
BMemorySystem::~BMemorySystem()
{
	CleanUpCallstackStats();
#if MEMORY_TRACKING >= 1
	Flush();
	if(m_startupAllocations != m_numAllocations)
	{
		ASSERT_RECOVERABLE(false, "Memory Leaks");
	}
	DebuggerPrintf("\n//=============================================================================================\n");
	DebuggerPrintf("Shut Down \n");
	DebuggerPrintf("Leaks: %u \n", m_numAllocations);
	DebuggerPrintf("Bytes Leaked: %u \n", m_totalAllocated);
	DebuggerPrintf("//=============================================================================================\n\n");
	DebuggerPrintf("Stack over time:");
	CallstackSystem::Shutdown();
#endif
	g_SkipTracking = true;
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::SystemUpdate()
{
	//Run once every second
	float elapsedTime = Time::TOTAL_SECONDS - m_timeStampOfPreviousAnalysis;
	if(elapsedTime >= 1.f)
	{
		m_timeStampOfPreviousAnalysis = Time::TOTAL_SECONDS;
		m_allocationsInTheLastSecond = m_allocationsForOneSecond;
		m_averageAllocationsPerSecond = (float)m_allocationsInTheLastSecond / elapsedTime;
		m_deallocationsInTheLastSecond = m_deallocationsForOneSecond;
		m_averageDeallocationsPerSecond = (float)m_deallocationsInTheLastSecond / elapsedTime;
		m_allocationsForOneSecond = 0;
		m_deallocationsForOneSecond = 0;
#if MEMORY_TRACKING >= 2
		PopulateCallstackStats();
#endif // MEMORY_TRACKING >= 2
	}
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::SystemGetMemoryAllocationString(std::string & allocationString)
{
#if MEMORY_TRACKING >= 1
	allocationString = Stringf("Allocations: %u | Bytes Allocated: %u | Most Bytes: %u",
		m_numAllocations,
		m_totalAllocated,
		m_highestTotalAllocated
	);
#elif MEMORY_TRACKING == 0
	allocationString = "No memory debug tracking.";
#endif // MEMORY_TRACKING >= 1
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::SystemGetMemoryAveragesString(std::string & averageString)
{
#if MEMORY_TRACKING >= 1
	averageString = Stringf("Average Allocations: %.1f | Average Deallocations: %.1f | Allocation Rate: %.1f",
		m_averageAllocationsPerSecond,
		m_averageDeallocationsPerSecond,
		(m_averageAllocationsPerSecond - m_averageDeallocationsPerSecond)
	);
#elif MEMORY_TRACKING == 0
	averageString = " ";
#endif // MEMORY_TRACKING >= 1
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::SystemFlush()
{
#if MEMORY_TRACKING >= 2
	PopulateCallstackStats(); //Calls CallstackGetLines()
	for(auto callstackStatsItem : m_callstackStatsMap)
	{
		DebuggerPrintf("\n//---------------------------------------------------------------------------------------------\n\n");
		CallstackStats & foundStats = callstackStatsItem.second;
		size_t leakedAllocations = foundStats.m_totalAllocations;
		size_t leakedBytes = foundStats.m_totalBytes;
		Callstack * currentCallstack = foundStats.m_callstackPtr;
		CallstackLine * lines = CallstackSystem::GetLines(currentCallstack); //And here it is twice
		DebuggerPrintf(Stringf("Allocations: %u | Bytes: %u\n", leakedAllocations, leakedBytes).c_str());
		for(unsigned int index = 0; index < currentCallstack->frame_count; ++index)
		{
			DebuggerPrintf(Stringf("%s\n%s(%u)\n", lines[index].function_name, lines[index].filename, lines[index].line).c_str());
		}
	}
#endif // MEMORY_TRACKING >= 2
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::PopulateCallstackStats()
{
	CleanUpCallstackStats();

	for(auto callstackItem : m_callstackMap)
	{
		s_MemorySystem->LockCallstackMap();
		size_t * leakedPtr = (size_t*)(callstackItem.first);
		--leakedPtr;
		size_t leakedBytes = *leakedPtr;
		Callstack * currentCallstack = callstackItem.second;
		s_MemorySystem->UnlockCallstackMap();

		//Hash allocation location
		uint32_t callstackHash = HashMemory(currentCallstack->frameDataPtr, currentCallstack->frame_count * sizeof(void *));

		//Find associated allocation location's allocation stats
		auto callstackStatsItem = m_callstackStatsMap.find(callstackHash);
		//Add it to map
		if(callstackStatsItem == m_callstackStatsMap.end())
		{
			CallstackStats newStats;
			newStats.m_callstackPtr = currentCallstack;
			newStats.m_totalAllocations = 1;
			newStats.m_totalBytes = leakedBytes;
			CallstackLine & topLine = CallstackSystem::GetTopLine(currentCallstack);
			std::string debugMemoryLine = Stringf("%s(%u)", topLine.filename, topLine.line);
			newStats.m_lineAndNumber = CreateNewCString(debugMemoryLine);
			m_callstackStatsMap.insert(std::pair<uint32_t, CallstackStats>(callstackHash, newStats));
		}
		//Update item in map
		else
		{
			CallstackStats & foundStats = callstackStatsItem->second;
			foundStats.m_totalAllocations += 1;
			foundStats.m_totalBytes += leakedBytes;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::CleanUpCallstackStats()
{
	for(auto callstackStatsItem : m_callstackStatsMap)
	{
		delete callstackStatsItem.second.m_lineAndNumber;
		callstackStatsItem.second.m_lineAndNumber = nullptr;
	}
	m_callstackStatsMap.clear();
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::LockCallstackMap()
{
	m_criticalSectionCallstackMap.Lock();
}


//-------------------------------------------------------------------------------------------------
void BMemorySystem::UnlockCallstackMap()
{
	m_criticalSectionCallstackMap.Unlock();
}
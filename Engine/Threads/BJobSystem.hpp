#pragma once

#include <atomic>
#include <vector>
#include "Engine/Threads/BQueue.hpp"
#include "Engine/Threads/Thread.hpp"
#include "Engine/MemorySystem/ObjectPool.hpp"
#include "Engine/Threads/Job.hpp"


//-------------------------------------------------------------------------------------------------
void JobSystemThreadEntry(void *);


//-------------------------------------------------------------------------------------------------
class JobConsumer
{
private:
	std::vector<eJobCategory> m_categories;

public:
	JobConsumer();

	void AddCategory(eJobCategory const & category);
	void ConsumeAll();
	bool Consume();
};


//-------------------------------------------------------------------------------------------------
class BJobSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static const int MAX_JOBS = 1024;
	static BJobSystem * s_System;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	std::vector<BQueue<Job*>*> m_jobQueue;
	std::vector<Thread> m_threads;
	//#TODO: Figure out how big to initialize it
	ObjectPool<Job> m_jobMemoryPool;
	bool m_isRunning;
	CriticalSection m_criticalSection;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup(int numOfThreads);
	static void Shutdown();
	static BJobSystem * CreateOrGetSystem();
	static size_t GetCoreCount();


	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BJobSystem();
	~BJobSystem();

	Job * JobCreate(eJobCategory const & category, JobCallback * jobFunc);
	void JobDispatch(Job * job);
	void JobDetach(Job * job);
	void JobJoin(Job * job);
	void Finish(Job * job);

	BQueue<Job*> * GetJobQueue(eJobCategory const & category) const;
	bool IsRunning() const;
};
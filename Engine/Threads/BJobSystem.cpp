#include "Engine/Threads/BJobSystem.hpp"

#include <thread>


//-------------------------------------------------------------------------------------------------
STATIC BJobSystem * BJobSystem::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
void JobSystemThreadEntry(void *)
{
	JobConsumer consumer;
	consumer.AddCategory(eJobCategory_GENERIC_SLOW);
	consumer.AddCategory(eJobCategory_GENERIC);
	while(BJobSystem::s_System && BJobSystem::s_System->IsRunning())
	{
		consumer.ConsumeAll();
		std::this_thread::yield();
	}

	// Make sure there is nothing left
	consumer.ConsumeAll();
}


//-------------------------------------------------------------------------------------------------
JobConsumer::JobConsumer()
{
	// Nothing
}


//-------------------------------------------------------------------------------------------------
void JobConsumer::AddCategory(eJobCategory const & category)
{
	m_categories.push_back(category);
}


//-------------------------------------------------------------------------------------------------
void JobConsumer::ConsumeAll()
{
	while(Consume());
}


//-------------------------------------------------------------------------------------------------
bool JobConsumer::Consume()
{
	for(eJobCategory const & category : m_categories)
	{
		Job * job;
		BQueue<Job*> * queue = BJobSystem::s_System->GetJobQueue(category);
		if(queue->PopFront(&job))
		{
			job->Work();
			BJobSystem::s_System->Finish(job);
			return true;
		}
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC void BJobSystem::Startup(int numOfThreads)
{
	if(s_System)
	{
		return;
	}

	//#TODO: Add a sleep(10) to the job threads?
	s_System = new BJobSystem();

	// Create number of threads = numOfThreads
	// Unless numOfThreads is negative, then assume it means (Max - number)
	if(numOfThreads < 0)
	{
		numOfThreads += GetCoreCount();
	}

	// But always make at least 1
	if(numOfThreads < 0)
	{
		numOfThreads = 1;
	}

	for(int threadIndex = 0; threadIndex < numOfThreads; ++threadIndex)
	{
		s_System->m_threads.push_back(Thread(JobSystemThreadEntry));
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BJobSystem::Shutdown()
{
	if(s_System)
	{
		delete s_System;
		s_System = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BJobSystem * BJobSystem::CreateOrGetSystem()
{
	if(!s_System)
	{
		// Going to assume (-2) is the default for now
		Startup(-2);
	}

	return s_System;
}


//-------------------------------------------------------------------------------------------------
STATIC size_t BJobSystem::GetCoreCount()
{
	return (size_t)std::thread::hardware_concurrency();
}


//-------------------------------------------------------------------------------------------------
BJobSystem::BJobSystem()
	: m_jobMemoryPool(MAX_JOBS, "m_jobMemoryPool")
	, m_isRunning(true)
{
	for(size_t jobCategoryIndex = 0; jobCategoryIndex < eJobCategory_COUNT; ++jobCategoryIndex)
	{
		m_jobQueue.push_back(new BQueue<Job*>());
	}
}


//-------------------------------------------------------------------------------------------------
BJobSystem::~BJobSystem()
{
	m_isRunning = false;

	for(size_t threadIndex = 0; threadIndex < m_threads.size(); ++threadIndex)
	{
		m_threads[threadIndex].Join();
	}

	m_criticalSection.Lock();
	m_jobMemoryPool.Destroy();
	m_criticalSection.Unlock();

	for(size_t jobCategoryIndex = 0; jobCategoryIndex < eJobCategory_COUNT; ++jobCategoryIndex)
	{
		delete m_jobQueue[jobCategoryIndex];
		m_jobQueue[jobCategoryIndex] = nullptr;
	}
	m_jobQueue.clear();
}


//-------------------------------------------------------------------------------------------------
Job * BJobSystem::JobCreate(eJobCategory const & category, JobCallback * jobFunc)
{
	m_criticalSection.Lock();
	Job * newJob = m_jobMemoryPool.Alloc();
	m_criticalSection.Unlock();

	newJob->m_category = category;
	newJob->m_jobFunc = jobFunc;
	++newJob->m_refCount;
	return newJob;
}


//-------------------------------------------------------------------------------------------------
void BJobSystem::JobDispatch(Job * job)
{
	++job->m_refCount;
	m_jobQueue[job->m_category]->PushBack(job);
}


//-------------------------------------------------------------------------------------------------
void BJobSystem::JobDetach(Job * job)
{
	--job->m_refCount;
	if(job->m_refCount == 0)
	{
		m_criticalSection.Lock();
		m_jobMemoryPool.Delete(job);
		m_criticalSection.Unlock();
	}
}


//-------------------------------------------------------------------------------------------------
void BJobSystem::JobJoin(Job * job)
{
	while(job->m_refCount == 2);
	--job->m_refCount;
	if(job->m_refCount == 0)
	{
		m_criticalSection.Lock();
		m_jobMemoryPool.Delete(job);
		m_criticalSection.Unlock();
	}
	else
	{
		ERROR_AND_DIE("Job still has a reference");
	}
}


//-------------------------------------------------------------------------------------------------
void BJobSystem::Finish(Job * job)
{
	--job->m_refCount;
	if(job->m_refCount == 0)
	{
		m_criticalSection.Lock();
		m_jobMemoryPool.Delete(job);
		m_criticalSection.Unlock();
	}
}


//-------------------------------------------------------------------------------------------------
BQueue<Job*> * BJobSystem::GetJobQueue(eJobCategory const & category) const
{
	return m_jobQueue[(size_t)category];
}


//-------------------------------------------------------------------------------------------------
bool BJobSystem::IsRunning() const
{
	return m_isRunning;
}

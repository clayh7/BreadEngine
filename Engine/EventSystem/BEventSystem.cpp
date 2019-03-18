#include "Engine/EventSystem/BEventSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include <algorithm>


//-------------------------------------------------------------------------------------------------
BEventSystem * BEventSystem::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC void BEventSystem::Startup()
{
	if(!s_System)
	{
		s_System = new BEventSystem();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BEventSystem::Shutdown()
{
	if(s_System)
	{
		delete s_System;
		s_System = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BEventSystem * BEventSystem::CreateOrGetSystem()
{
	if(!s_System)
	{
		Startup();
	}

	return s_System;
}


//-------------------------------------------------------------------------------------------------
STATIC void BEventSystem::RegisterEventAndCommand(std::string const & eventName, std::string const & usage, EventCallback * callback, int priority /*= 0*/)
{
	BConsoleSystem::Register(eventName, callback, usage);
	RegisterEvent(eventName, callback, priority);
}


//-------------------------------------------------------------------------------------------------
void BEventSystem::RegisterEvent(std::string const & eventName, EventCallback * callback, int priority /*= 0*/)
{
	if(!s_System)
	{
		return;
	}

	SubscriberMap & subscribers = s_System->m_registeredSubscribers;

	//Find the list of subscriptions under this name
	size_t eventNameHash = std::hash<std::string>{}(eventName);
	auto foundEventSubscription = subscribers.find(eventNameHash);

	//Create subscriber
	SubscriberStaticFunction * subscriber = new SubscriberStaticFunction();
	subscriber->m_priority = priority;
	subscriber->m_function = callback;

	//If subscription exists, add to it
	if(foundEventSubscription != subscribers.end())
	{
		std::vector<SubscriberBase*> & eventSubscription = foundEventSubscription->second;
		// Don't allow duplicate static function subscribers
		for(size_t index = 0; index < eventSubscription.size(); ++index)
		{
			SubscriberStaticFunction const * currentSub = dynamic_cast<SubscriberStaticFunction*>(eventSubscription[index]);
			if(currentSub && currentSub->m_function == callback)
			{
				delete subscriber;
				return;
			}
		}
		eventSubscription.push_back(subscriber);
	}

	//If subscription does not exist yet, create one
	else
	{
		std::vector<SubscriberBase*> eventSubscription;
		eventSubscription.push_back(subscriber);
		subscribers.insert(std::pair<size_t, std::vector<SubscriberBase*>>(eventNameHash, eventSubscription));
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BEventSystem::TriggerEvent(std::string const & eventName)
{
	NamedProperties empty = NamedProperties();
	TriggerEvent(eventName, empty);
}


//-------------------------------------------------------------------------------------------------
STATIC void BEventSystem::TriggerEvent(std::string const & eventName, NamedProperties & eventData)
{
	size_t eventNameHash = std::hash<std::string>{}(eventName);
	SubscriberMap & subscribers = s_System->m_registeredSubscribers;

	// Event exists
	auto foundEventSubscription = subscribers.find(eventNameHash);
	if(foundEventSubscription != subscribers.end())
	{
		std::vector<SubscriberBase*> & eventSubscription = foundEventSubscription->second;
		// Execute in order of priority, same priority can have random order
		std::sort(eventSubscription.begin(), eventSubscription.end(), [](SubscriberBase* a, SubscriberBase* b)
		{
			return a->m_priority > b->m_priority;
		});
		for(SubscriberBase const * subscriber : eventSubscription)
		{
			subscriber->Execute(eventData);
		}
	}

	// Event does not exist, so do nothing
	else
	{
		return;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BEventSystem::TriggerEventForFilesFound(std::string const & eventName, std::string const & baseFolder, std::string const & filePattern)
{
	std::vector < std::string > filesFound = EnumerateFilesInFolder(baseFolder, filePattern);
	for(std::string & relativePath : filesFound)
	{
		NamedProperties fileFoundEvent;
		size_t const MAX_PATH = 260;
		char absolutePath[MAX_PATH];
		_fullpath(absolutePath, relativePath.c_str(), MAX_PATH);

		std::vector<std::string> seperatedPath = SplitString(relativePath, '/');
		std::string fileName = seperatedPath.back();
		std::vector<size_t> periodPosition = FindIndicies(fileName, '.');
		std::vector<std::string> seperatedAbsolutePath = SplitString(absolutePath, '\\');
		std::string builtAbsolutePath;
		for(std::string & absolutePart : seperatedAbsolutePath)
		{
			builtAbsolutePath += absolutePart;
			builtAbsolutePath += '/';
		}
		builtAbsolutePath.pop_back();

		fileFoundEvent.Set("FileName", fileName);
		fileFoundEvent.Set("FileExtension", fileName.substr(periodPosition.back()));
		fileFoundEvent.Set("FileNameWithoutExtension", fileName.substr(0, periodPosition.back()));
		fileFoundEvent.Set("FileRelativePath", relativePath);
		fileFoundEvent.Set("FileAbsolutePath", builtAbsolutePath);

		TriggerEvent(eventName, fileFoundEvent);
	}
}


//-------------------------------------------------------------------------------------------------
BEventSystem::BEventSystem()
	: m_registeredSubscribers()
{
	// Nothing
}


//-------------------------------------------------------------------------------------------------
BEventSystem::~BEventSystem()
{
	for(auto & subscriptions : m_registeredSubscribers)
	{
		for(auto deleteMe : subscriptions.second)
		{
			delete deleteMe;
			deleteMe = nullptr;
		}
	}
	m_registeredSubscribers.clear();
}

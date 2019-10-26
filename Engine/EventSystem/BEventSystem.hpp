#pragma once

#include <string>
#include <map>
#include <vector>
#include "Engine/DebugSystem/BConsoleSystem.hpp"


//-------------------------------------------------------------------------------------------------
class NamedProperties;
typedef void (EventCallback)(NamedProperties &);


//-----------------------------------------------------------------------------------------------
class SubscriberBase
{
public:
	int m_priority;

public:
	virtual void Execute(NamedProperties &) const = 0;
	virtual void * GetObject() const = 0;
};
typedef std::map<size_t, std::vector<SubscriberBase*>> SubscriberMap;


//-------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_FunctionType>
class SubscriberObjectFunction : public SubscriberBase
{
public:
	T_ObjectType * m_object;
	T_FunctionType m_function;

public:
	virtual void Execute(NamedProperties & params) const override
	{
		(m_object->*m_function)(params);
	}

private:
	virtual void * GetObject() const
	{
		return (void*)m_object;
	}
};


//-------------------------------------------------------------------------------------------------
class SubscriberStaticFunction : public SubscriberBase
{
public:
	EventCallback * m_function;

public:
	virtual void Execute(NamedProperties & params) const override
	{
		(m_function)(params);
	}

private:
	virtual void * GetObject() const
	{
		return nullptr;
	}
};


//-----------------------------------------------------------------------------------------------
class BEventSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static BEventSystem * s_System;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	SubscriberMap m_registeredSubscribers;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static BEventSystem * CreateOrGetSystem();
	static void RegisterEventAndCommand(std::string const & eventName, std::string const & usage, EventCallback * callback, int priority = 0);
	static void RegisterEvent(std::string const & eventName, EventCallback * callback, int priority = 0);
	static void TriggerEvent(std::string const & eventName);
	static void TriggerEvent(std::string const & eventName, NamedProperties & eventData);
	static void TriggerEventForFilesFound(std::string const & eventName, std::string const & baseFolder, std::string const & filePattern);

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BEventSystem();
	~BEventSystem();

	//-------------------------------------------------------------------------------------------------
	// Static Function Templates
	//-------------------------------------------------------------------------------------------------
public:
	template <typename T_ObjectType, typename T_FunctionType>
	static void RegisterEventAndCommand(std::string const & eventName, std::string const & usage, T_ObjectType * object, T_FunctionType function, int priority = 0)
	{
		BConsoleSystem::Register(eventName, object, function, usage);
		RegisterEvent(eventName, object, function, priority);
	}

	template <typename T_ObjectType, typename T_FunctionType>
	static void RegisterEvent(std::string const & eventName, T_ObjectType * object, T_FunctionType function, int priority = 0)
	{
		BEventSystem * system = BEventSystem::CreateOrGetSystem();
		SubscriberMap & subscribers = system->m_registeredSubscribers;

		//Find the list of subscriptions under this name
		size_t eventNameHash = std::hash<std::string>{}(eventName);
		auto foundEventSubscription = subscribers.find(eventNameHash);

		//Create subscriber
		SubscriberObjectFunction<T_ObjectType, T_FunctionType> * subscriber = new SubscriberObjectFunction<T_ObjectType, T_FunctionType>();
		subscriber->m_priority = priority;
		subscriber->m_object = object;
		subscriber->m_function = function;

		//If subscription exists, add to it
		if(foundEventSubscription != subscribers.end())
		{
			std::vector<SubscriberBase*> & eventSubscription = foundEventSubscription->second;
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

	//Remove the subscriber from all of their Registered Events
	template <typename T_ObjectType>
	static void Unregister(T_ObjectType * subscriber)
	{
		BEventSystem * system = BEventSystem::s_System;
		if(!system)
		{
			return;
		}
		SubscriberMap & subscribers = system->m_registeredSubscribers;

		for(auto & eventSubscriptionPair : subscribers)
		{
			std::vector<SubscriberBase*> & eventSubscription = eventSubscriptionPair.second;
			for(auto subscriberIter = eventSubscription.begin(); subscriberIter != eventSubscription.end(); /*Nothing*/)
			{
				SubscriberBase * subscriberBase = *subscriberIter;
				T_ObjectType * object = static_cast<T_ObjectType*>(subscriberBase->GetObject());

				//Make sure subscriber is the correct type
				if(object)
				{
					//Make sure subscriber matches our criteria
					if(object == subscriber)
					{
						//Remove the subscriber
						delete subscriberBase;
						subscriberIter = eventSubscription.erase(subscriberIter);
					}
					else
					{
						++subscriberIter;
					}
				}
				else
				{
					++subscriberIter;
				}
			}
		}
	}
};
#include "Engine/InputSystem/BInputSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/InputSystem/BMouseKeyboard.hpp"
#include "Engine/InputSystem/BXboxController.hpp"


//-------------------------------------------------------------------------------------------------
STATIC BInputSystem * BInputSystem::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC void BInputSystem::Startup()
{
	if(!s_System)
	{
		s_System = new BInputSystem();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BInputSystem::Shutdown()
{
	if(s_System)
	{
		delete s_System;
		s_System = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BInputSystem * BInputSystem::CreateOrGetSystem()
{
	if(!s_System)
	{
		Startup();
	}

	return s_System;
}

//-------------------------------------------------------------------------------------------------
BInputSystem::BInputSystem()
{
	BEventSystem::RegisterEvent(EVENT_ENGINE_UPDATE, this, &BInputSystem::OnUpdate, 10);
	BMouseKeyboard::s_Instance = new BMouseKeyboard();
	BXboxController::s_Instance = new BXboxController();
}


//-------------------------------------------------------------------------------------------------
BInputSystem::~BInputSystem()
{
	BEventSystem::Unregister(this);
	if(BMouseKeyboard::s_Instance)
	{
		delete BMouseKeyboard::s_Instance;
		BMouseKeyboard::s_Instance = nullptr;
	}
	if(BXboxController::s_Instance)
	{
		delete BXboxController::s_Instance;
		BXboxController::s_Instance = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void BInputSystem::OnUpdate(NamedProperties &)
{
	BMouseKeyboard::Update();
	BXboxController::Update();
}
#include "Engine/InputSystem/BInputSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
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
STATIC void BInputSystem::Update()
{
	if(s_System)
	{
		s_System->SystemUpdate();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BInputSystem * BInputSystem::GetSystem()
{
	return s_System;
}

//-------------------------------------------------------------------------------------------------
BInputSystem::BInputSystem()
{
	BMouseKeyboard::s_Instance = new BMouseKeyboard();
	BXboxController::s_Instance = new BXboxController();
}


//-------------------------------------------------------------------------------------------------
BInputSystem::~BInputSystem()
{
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
void BInputSystem::SystemUpdate()
{
	BMouseKeyboard::Update();
	BXboxController::Update();
}
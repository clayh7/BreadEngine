#include "Game/App.hpp"

#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/InputSystem/BMouseKeyboard.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Game/Game.hpp"
#define STATIC


//-------------------------------------------------------------------------------------------------
App * g_AppSystem = nullptr;


//-------------------------------------------------------------------------------------------------
App::App()
{
	BConsoleSystem::AddLog(Stringf("Project: %s", APP_NAME), BConsoleSystem::INFO);

	//Create Game
	g_GameSystem = new Game();
}


//-------------------------------------------------------------------------------------------------
App::~App()
{
	delete g_GameSystem;
	g_GameSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
void App::Update()
{
	BProfiler::StartSample("UPDATE GAME");

	UpdateInputs();
	g_GameSystem->Update();

	BProfiler::StopSample();
}


//-------------------------------------------------------------------------------------------------
void App::UpdateInputs()
{
	//Quitting App
	if(BMouseKeyboard::GetKeyDown(eKeyboardButton_ESCAPE))
	{
		BConsoleSystem * CSystem = BConsoleSystem::s_System;
		if(CSystem)
		{
			if(!CSystem->IsVisible())
			{
				g_isQuitting = true;
			}
		}
		else
		{
			g_isQuitting = true;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void App::Render() const
{
	BProfiler::StartSample("RENDER GAME");

	//Draw Game
	g_GameSystem->Render();

	BProfiler::StopSample();
}
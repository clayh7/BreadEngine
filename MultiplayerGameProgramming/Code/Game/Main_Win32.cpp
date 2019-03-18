#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Core/Engine.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/MemorySystem/BMemorySystem.hpp"
#include "Game/App.hpp"


//-------------------------------------------------------------------------------------------------
void Update()
{
	BProfiler::StartSample("UPDATE");

	g_EngineSystem->Update(); //Needs to run first to update Input/Audio/etc...
	g_AppSystem->Update();
	g_EngineSystem->LateUpdate();

	BProfiler::StopSample();
}


//-------------------------------------------------------------------------------------------------
void Render()
{
	BProfiler::StartSample("RENDER");

	//Clear Screen
	BRenderSystem::ClearScreen(Color::CLAY_GREEN);

	//Print world
	g_AppSystem->Render();
	g_EngineSystem->Render(); //Needs to run second to print on top of App

	BProfiler::StopSample();
}


//-------------------------------------------------------------------------------------------------
void RunFrame()
{
	BProfiler::Update();
	Update();
	Render();
}


//-------------------------------------------------------------------------------------------------
void Startup(HINSTANCE applicationInstanceHandle)
{
	//Engine and Game Initialization
	g_EngineSystem = new(eMemoryTag_UNTRACKED) Engine(applicationInstanceHandle);
	g_AppSystem = new App();
}


//-------------------------------------------------------------------------------------------------
void Shutdown()
{
	//Destroy in inverse order
	delete g_AppSystem;
	g_AppSystem = nullptr;

	delete g_EngineSystem;
	g_EngineSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR, int)
{
	Startup(applicationInstanceHandle);

	while(!g_isQuitting)
	{
		RunFrame();
	}

	Shutdown();
	return 0;
}
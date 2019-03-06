#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <cassert>
#include <crtdbg.h>
#include <time.h>
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library

#include "Engine/Core/Engine.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Game/App.hpp"


//-------------------------------------------------------------------------------------------------
void Update( )
{
	g_ProfilerSystem->StartSample( "UPDATE" );

	g_EngineSystem->Update( ); //Needs to run first to update Input/Audio/etc...
	g_AppSystem->Update( );

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void Render( )
{
	g_ProfilerSystem->StartSample( "RENDER" );

	//Clear Screen
	g_RenderSystem->ClearScreen( Color::BLACK );

	//Print world
	g_AppSystem->Render( );
	g_EngineSystem->Render( ); //Needs to run second to print on top of App

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void RunFrame( )
{
	g_ProfilerSystem->FrameMark( );
	Update( );
	Render( );
}


//-------------------------------------------------------------------------------------------------
void Initialize( HINSTANCE applicationInstanceHandle )
{
	//Engine and Game Initialization
	g_EngineSystem = new Engine( applicationInstanceHandle );
	g_AppSystem = new App( );
}


//-------------------------------------------------------------------------------------------------
void Shutdown( )
{
	//Destroy in inverse order
	delete g_AppSystem;
	g_AppSystem = nullptr;

	delete g_EngineSystem;
	g_EngineSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR, int )
{
	Initialize( applicationInstanceHandle );

	while ( !g_isQuitting )
	{
		RunFrame( );
	}

	Shutdown( );
	return 0;
}
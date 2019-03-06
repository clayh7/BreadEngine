#include "Game/App.hpp"

#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Game/Game.hpp"
#define STATIC


//-------------------------------------------------------------------------------------------------
App * g_AppSystem = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC char const * App::APP_NAME = "ProtoGame"; //Name on top game window


//-------------------------------------------------------------------------------------------------
App::App( )
{
	//Startup text
	g_ConsoleSystem->AddLog( "Engine: Bread v1.0.0", Console::INFO );
	g_ConsoleSystem->AddLog( Stringf( "Project: %s", APP_NAME ), Console::INFO );
	g_ConsoleSystem->AddLog( "Author: Clay Howell", Console::INFO );

	//Create Game
	g_GameSystem = new Game();
}


//-------------------------------------------------------------------------------------------------
App::~App( )
{
	delete g_GameSystem;
	g_GameSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
void App::Update( )
{
	g_ProfilerSystem->StartSample( "UPDATE GAME" );

	UpdateInputs( );
	g_GameSystem->Update( );

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void App::UpdateInputs( )
{
	//Quitting App
	if ( g_InputSystem->WasKeyJustPressed( Input::KEY_ESCAPE ) )
	{
		g_isQuitting = true;
	}
}


//-------------------------------------------------------------------------------------------------
void App::Render( ) const
{
	g_ProfilerSystem->StartSample( "RENDER GAME" );

	//Clear screen
	g_RenderSystem->ClearScreen( Color::CLAY_GREEN );

	//Draw Game
	g_GameSystem->Render( );

	g_ProfilerSystem->StopSample( );
}
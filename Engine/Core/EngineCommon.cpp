#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Engine.hpp"
#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------
bool g_isQuitting = false;
bool g_limitFPS = true;
int DRAW_CALLS = 0;
char const * const EVENT_ENGINE_UPDATE = "NetworkUpdate";
char const * const EVENT_ENGINE_RENDER = "EngineRender";


//-------------------------------------------------------------------------------------------------
// Global Functions
//-------------------------------------------------------------------------------------------------
Vector2i GetWindowDimensions()
{
	return Engine::GetWindowDimensions();
}
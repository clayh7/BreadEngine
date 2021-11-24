#include "Game/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
// Global Events
//-------------------------------------------------------------------------------------------------
const char* const EVENT_GAME_UPDATE = "GameUpdate";
const char* const EVENT_GAME_UPDATE_LATE = "GameUpdateLate";
const char* const EVENT_GAME_RENDER = "GameRender";
const char* const EVENT_GAME_SHUTDOWN = "GameShutDown";


//-------------------------------------------------------------------------------------------------
Vector2f GameCommon::CoordsToWorld(Coords input)
{
	return Vector2f(input.x*16.f, input.y*16.f);
}

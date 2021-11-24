#pragma once

#include <vector>
#include <unordered_map>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
// Global Events
//-------------------------------------------------------------------------------------------------
extern char const * const EVENT_GAME_UPDATE;
extern char const * const EVENT_GAME_UPDATE_LATE;
extern char const * const EVENT_GAME_RENDER;
extern char const * const EVENT_GAME_SHUTDOWN;


namespace GameCommon
{
	//-------------------------------------------------------------------------------------------------
	// Common Types
	//-------------------------------------------------------------------------------------------------
	enum class Direction
	{
		North,
		South,
		East,
		West,
	};

	typedef Vector2i Coords;

	enum class GameObjectType
	{
		Empty,
		Starter,
		Ender,
		Pusher,
	};


	//-------------------------------------------------------------------------------------------------
	// Common Functions
	//-------------------------------------------------------------------------------------------------
	Vector2f CoordsToWorld(Coords input);
}

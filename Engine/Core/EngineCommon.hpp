#pragma once

#include <string>
#include "Engine/Core/BuildConfig.hpp"
#define STATIC // The only purpose of this is to remind people in the .cpp that a function or variable is static
#define BIT(x)(1<<(x))
#define UNREFERENCED(x) x
class Vector2i;
typedef unsigned char byte_t;
typedef float float32_t;
typedef double double64_t;


//-------------------------------------------------------------------------------------------------
// Global Members
//-------------------------------------------------------------------------------------------------
extern bool g_isQuitting;
extern bool g_limitFPS;
extern char const * const EVENT_ENGINE_UPDATE;
extern char const * const EVENT_ENGINE_RENDER;


//-------------------------------------------------------------------------------------------------
// Global Functions
//-------------------------------------------------------------------------------------------------
Vector2i GetWindowDimensions();


//-------------------------------------------------------------------------------------------------
// Engine Enumerators
//-------------------------------------------------------------------------------------------------
enum eBlending
{
	eBlending_NORMAL,
	eBlending_SUBTRACTIVE,
	eBlending_ADDITIVE,
	eBlending_INVERTED,
};


//-------------------------------------------------------------------------------------------------
enum eDrawMode
{
	eDrawMode_FULL,
	eDrawMode_LINE,
	eDrawMode_POINT,
};


//-------------------------------------------------------------------------------------------------
enum ePrimitiveType
{
	ePrimitiveType_POINTS,
	ePrimitiveType_LINES,
	ePrimitiveType_TRIANGLES,
	ePrimitiveType_TRIANGLE_FAN,
	ePrimitiveType_QUADS,
	ePrimitiveType_COUNT,
};
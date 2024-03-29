#pragma once

#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
class AABB2i
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static AABB2i const ZERO_TO_ZERO;
	static AABB2i const ZERO_TO_ONE;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	Vector2i mins;
	Vector2i maxs;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	AABB2i();
	AABB2i(Vector2i const & setMins, Vector2i const & setMaxs);
	~AABB2i();
};
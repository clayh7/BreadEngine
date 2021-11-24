#pragma once

#include "GameCommon.hpp"
#include "GameObject.hpp"

class GameObject;

class Slot
{
public:
	Coords m_coords;
	GameObjectPtr m_floorObject;
	GameObjectPtr m_occupyingObject;

public:
	Slot();
	void SetFloor(GameObjectPtr gameObject);
	void SetOccupying(GameObjectPtr gameObject);
};
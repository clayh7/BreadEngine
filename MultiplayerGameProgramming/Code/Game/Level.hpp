#pragma once

#include "GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class Slot;


//-------------------------------------------------------------------------------------------------
class Level
{
public:
	std::vector<std::vector<Slot>> m_grid;

public:
	Level() = delete;
	Level(int width, int height);
	~Level();

	void SetupDemo();
	void SetFloor(int xPos, int yPos, GameCommon::GameObjectType type);
	void OnUpdate(NamedProperties &);
	void OnRender(NamedProperties &) const;
};
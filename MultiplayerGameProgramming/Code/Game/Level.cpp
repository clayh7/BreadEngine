#include "Game/Level.hpp"

#include "Game/Slot.hpp"


//-------------------------------------------------------------------------------------------------
Level::Level(int width, int height)
{
	m_grid = std::vector<std::vector<Slot>>(width, std::vector<Slot>(height));

	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			Slot& slot = m_grid[x][y];
			slot.m_coords = Vector2i(x, y);
		}
	}

	BEventSystem::RegisterEvent(EVENT_GAME_UPDATE, this, &Level::OnUpdate);
	BEventSystem::RegisterEvent(EVENT_GAME_RENDER, this, &Level::OnRender);
}


//-------------------------------------------------------------------------------------------------
Level::~Level()
{

}


//-------------------------------------------------------------------------------------------------
void Level::SetupDemo()
{
	SetFloor(2, 2, GameObjectType::Starter);
	SetFloor(4, 4, GameObjectType::Ender);
}


//-------------------------------------------------------------------------------------------------
void Level::SetFloor(int xPos, int yPos, GameObjectType type)
{
	GameObjectPtr created = GameObject::Spawn(type);
	m_grid[xPos][yPos].SetFloor(created);
}


//-------------------------------------------------------------------------------------------------
void Level::OnUpdate(NamedProperties &)
{

}


//-------------------------------------------------------------------------------------------------
void Level::OnRender(NamedProperties &) const
{

}

#include "Game/GameObject.hpp"

#include "Engine/RenderSystem/SpriteRenderSystem/BSpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"


//-------------------------------------------------------------------------------------------------
std::unordered_map<GameObjectType, RegisterGameObjectCallback*>* RegisterGameObject::s_registerMap = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC GameObjectPtr GameObject::Spawn(GameObjectType type)
{
	return RegisterGameObject::Spawn(type);
}


//-------------------------------------------------------------------------------------------------
GameObject::GameObject()
	: m_direction(Direction::North)
	, m_coords(0, 0)
	, m_order(0)
	, m_sprite("square")
{
	m_sprite.SetScale(0.1f);
	BEventSystem::RegisterEvent(EVENT_GAME_UPDATE, this, &GameObject::OnUpdate);
	BEventSystem::RegisterEvent(EVENT_GAME_RENDER, this, &GameObject::OnRender);
}


//-------------------------------------------------------------------------------------------------
void GameObject::OnUpdate(NamedProperties &)
{
	Vector2f position = GameCommon::CoordsToWorld(m_coords);
	m_sprite.SetPosition(position);
}


//-------------------------------------------------------------------------------------------------
void GameObject::OnRender(NamedProperties &) const
{
	
}


//-------------------------------------------------------------------------------------------------
STATIC GameObjectPtr RegisterGameObject::Spawn(GameObjectType type)
{
	if (s_registerMap->find(type) == s_registerMap->end())
	{
		return nullptr;
	}

	RegisterGameObjectCallback* callback = (*s_registerMap)[type];
	return callback();
}


//-------------------------------------------------------------------------------------------------
STATIC void RegisterGameObject::Cleanup(NamedProperties &)
{
	delete s_registerMap;
	s_registerMap = nullptr;
}


//-------------------------------------------------------------------------------------------------
RegisterGameObject::RegisterGameObject(GameObjectType type, RegisterGameObjectCallback* callback)
{
	if (s_registerMap == nullptr)
	{
		s_registerMap = new std::unordered_map<GameObjectType, RegisterGameObjectCallback*>();
		BEventSystem::RegisterEvent(EVENT_GAME_SHUTDOWN, &RegisterGameObject::Cleanup);
	}

	(*s_registerMap)[type] = callback;
}
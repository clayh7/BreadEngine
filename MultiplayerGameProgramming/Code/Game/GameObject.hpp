#pragma once

#include "GameCommon.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"


//-------------------------------------------------------------------------------------------------
class GameObject;
class Sprite;
typedef std::shared_ptr<GameObject> GameObjectPtr;
using namespace GameCommon;


//-------------------------------------------------------------------------------------------------
class GameObject
{
public:
	Direction m_direction;
	Coords m_coords;
	int m_order;
	Sprite m_sprite;

public:
	static GameObjectPtr Spawn(GameObjectType type);

public:
	GameObject();
	virtual void OnUpdate(NamedProperties &);
	virtual void OnRender(NamedProperties &) const;
};


//-------------------------------------------------------------------------------------------------
typedef GameObjectPtr (RegisterGameObjectCallback)();


//-------------------------------------------------------------------------------------------------
struct RegisterGameObject
{
public:
	friend GameObject;

private:
	static std::unordered_map<GameObjectType, RegisterGameObjectCallback*>* s_registerMap;
	static GameObjectPtr Spawn(GameObjectType type);
	static void Cleanup(NamedProperties &);

public:
	RegisterGameObject() = delete;
	RegisterGameObject(GameObjectType type, RegisterGameObjectCallback* callback);
};

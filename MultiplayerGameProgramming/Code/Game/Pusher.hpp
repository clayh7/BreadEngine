#pragma once
#include "GameObject.hpp"
#include "GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class Pusher : public GameObject
{
private:
	static RegisterGameObject s_pusherGameObject;
	static GameObjectPtr CreateThis();

public:
	Pusher();
};
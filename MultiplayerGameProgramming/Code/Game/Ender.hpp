#pragma once
#include "GameObject.hpp"
#include "GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class Ender : public GameObject
{
private:
	static RegisterGameObject s_enderGameObject;
	static GameObjectPtr CreateThis();

public:
	Ender();
};
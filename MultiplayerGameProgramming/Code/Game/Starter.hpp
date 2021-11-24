#pragma once
#include "GameObject.hpp"
#include "GameCommon.hpp"



//-------------------------------------------------------------------------------------------------
class Starter : public GameObject
{
private:
	static RegisterGameObject s_starterGameObject;
	static GameObjectPtr CreateThis();

public:
	Starter();
};
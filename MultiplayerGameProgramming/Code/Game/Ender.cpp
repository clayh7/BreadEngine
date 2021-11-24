#include "Ender.hpp"


//-------------------------------------------------------------------------------------------------
RegisterGameObject Ender::s_enderGameObject(GameObjectType::Ender, &Ender::CreateThis);


//-------------------------------------------------------------------------------------------------
GameObjectPtr Ender::CreateThis()
{
	return std::shared_ptr<Ender>(new Ender());
}


//-------------------------------------------------------------------------------------------------
Ender::Ender()
{

}


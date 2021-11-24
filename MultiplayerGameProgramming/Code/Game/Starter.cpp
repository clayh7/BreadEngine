#include "Starter.hpp"


//-------------------------------------------------------------------------------------------------
RegisterGameObject Starter::s_starterGameObject(GameObjectType::Starter, &Starter::CreateThis);


//-------------------------------------------------------------------------------------------------
GameObjectPtr Starter::CreateThis()
{
	return std::shared_ptr<Starter>(new Starter());
}


//-------------------------------------------------------------------------------------------------
Starter::Starter()
{

}


#include "Pusher.hpp"


//-------------------------------------------------------------------------------------------------
RegisterGameObject Pusher::s_pusherGameObject(GameObjectType::Pusher, &Pusher::CreateThis);


//-------------------------------------------------------------------------------------------------
GameObjectPtr Pusher::CreateThis()
{
	return std::shared_ptr<Pusher>(new Pusher());
}


//-------------------------------------------------------------------------------------------------
Pusher::Pusher()
{

}


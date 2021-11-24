#include "Engine/RenderSystem/SpriteRenderSystem/Particle.hpp"


//-------------------------------------------------------------------------------------------------
Particle::Particle(std::string const & spriteID, int layer, bool ignoreView)
	: sprite(spriteID, layer, ignoreView)
	, velocity(Vector2f::ZERO)
	, acceleration(Vector2f::ZERO)
	, age(0.f)
	, maxAge(0.f)
{
}


//-------------------------------------------------------------------------------------------------
Particle::~Particle()
{

}


//-------------------------------------------------------------------------------------------------
void Particle::SetPosition(Vector2f const & position)
{
	sprite.SetPosition(position);
}

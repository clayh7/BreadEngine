#include "Slot.hpp"

//-------------------------------------------------------------------------------------------------
Slot::Slot()
	: m_floorObject(nullptr)
	, m_occupyingObject(nullptr)
{

}


//-------------------------------------------------------------------------------------------------
void Slot::SetFloor(GameObjectPtr gameObject)
{
	m_floorObject = gameObject;
	if (m_floorObject)
	{
		m_floorObject->m_coords = m_coords;
	}
}


//-------------------------------------------------------------------------------------------------
void Slot::SetOccupying(GameObjectPtr gameObject)
{
	m_occupyingObject = gameObject;
	if (m_occupyingObject)
	{
		m_occupyingObject->m_coords = m_coords;
	}
}


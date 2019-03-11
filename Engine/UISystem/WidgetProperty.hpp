#pragma once

#include "Engine/UISystem/UICommon.hpp"


//-------------------------------------------------------------------------------------------------
template<typename Type>
class WidgetProperty
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	Type m_value;
	eWidgetPropertySource m_source;
	eWidgetState m_state;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	WidgetProperty(Type const & value = Type(), eWidgetPropertySource source = eWidgetPropertySource_CODE, eWidgetState state = eWidgetState_ALL)
		: m_value(value)
		, m_source(source)
		, m_state(state)
	{
		//Nothing
	}
};
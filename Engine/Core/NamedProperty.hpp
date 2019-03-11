#pragma once


//-------------------------------------------------------------------------------------------------
// Enums
//-------------------------------------------------------------------------------------------------
enum ePropertyGetResult
{
	ePropertyGetResult_SUCCESS,
	ePropertyGetResult_FAILED_WRONG_TYPE,
	ePropertyGetResult_FAILED_NOT_PRESENT,
};

enum ePropertySetResult
{
	ePropertySetResult_SUCCESS_ADDED,
	ePropertySetResult_SUCCESS_UPDATE,
	ePropertySetResult_SUCCESS_CHANGED_TYPE,
};


//-------------------------------------------------------------------------------------------------
class NamedPropertyBase
{
public:
	virtual ~NamedPropertyBase();
};


//-------------------------------------------------------------------------------------------------
template<typename Type>
class NamedProperty : public NamedPropertyBase
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	Type m_data;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	NamedProperty(Type const & data)
		: m_data(data)
	{
		//Nothing
	}

	virtual ~NamedProperty() override
	{
		//Nothing
	}
};
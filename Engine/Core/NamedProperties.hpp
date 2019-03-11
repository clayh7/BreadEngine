#pragma once

#include <map>
#include "Engine/Core/NamedProperty.hpp"


//-------------------------------------------------------------------------------------------------
class NamedProperties
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	std::map< size_t, NamedPropertyBase* > m_properties;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	NamedProperties();
	~NamedProperties();

	//-------------------------------------------------------------------------------------------------
	// Function Templates
	//-------------------------------------------------------------------------------------------------
public:
	template<typename T_type1>
	NamedProperties(std::string const & propertyName1, T_type1 const & propertyValue1)
	{
		Set(propertyName1, propertyValue1);
	}

	template<typename T_type1, typename T_type2>
	NamedProperties(std::string const & propertyName1, T_type1 const & propertyValue1,
		std::string const & propertyName2, T_type2 const & propertyValue2)
		: NamedProperties(propertyName1, propertyValue1)
	{
		Set(propertyName2, propertyValue2);
	}

	template<typename T_type1, typename T_type2, typename T_type3>
	NamedProperties(std::string const & propertyName1, T_type1 const & propertyValue1,
		std::string const & propertyName2, T_type2 const & propertyValue2,
		std::string const & propertyName3, T_type3 const & propertyValue3)
		: NamedProperties(propertyName1, propertyValue1, propertyName2, propertyValue2)
	{
		Set(propertyName3, propertyValue3);
	}

	template<typename Type>
	ePropertyGetResult Get(std::string const & propertyName, Type & out_propertyValue) const
	{
		//If we don't specify a default value, then we are the default value
		return Get(propertyName, out_propertyValue, out_propertyValue);
	}

	template<typename Type>
	ePropertyGetResult Get(std::string const & propertyName, Type & out_propertyValue, Type const & defaultValue) const
	{
		size_t propertyNameHash = std::hash<std::string>{}(propertyName);
		auto found = m_properties.find(propertyNameHash);

		//Could not find property
		if(found == m_properties.end())
		{
			return ePropertyGetResult_FAILED_NOT_PRESENT;
		}

		//Found property
		else
		{
			//Try to cast to the propertyValue type, if it's not the propertyValue type this'll return null
			NamedPropertyBase * property = found->second;
			NamedProperty<Type> * typedProperty = dynamic_cast<NamedProperty<Type>*>(property);

			//Property is correct type
			if(typedProperty)
			{
				out_propertyValue = typedProperty->m_data;
				return ePropertyGetResult_SUCCESS;
			}

			//Property is not correct type
			else
			{
				out_propertyValue = defaultValue;
				return ePropertyGetResult_FAILED_WRONG_TYPE;
			}
		}
	}

	//Specific Override
	ePropertySetResult Set(std::string const & propertyName, char * propertyValue);
	ePropertySetResult Set(std::string const & propertyName, char const * propertyValue);

	template<typename Type>
	ePropertySetResult Set(std::string const & propertyName, Type const & propertyValue)
	{
		size_t propertyNameHash = std::hash<std::string>{} (propertyName);
		auto found = m_properties.find(propertyNameHash);

		//Could not find property
		if(found == m_properties.end())
		{
			NamedProperty<Type> * addProperty = new NamedProperty<Type>(propertyValue);
			m_properties.insert(std::pair<size_t, NamedPropertyBase*>(propertyNameHash, addProperty));
			return ePropertySetResult_SUCCESS_ADDED;
		}

		//Found property
		else
		{
			//Try to cast to the propertyValue type, if it's not the propertyValue type this'll return null
			NamedPropertyBase * property = found->second;
			NamedProperty<Type> * typedProperty = dynamic_cast<NamedProperty<Type>*>(property);

			//Property is correct type
			if(typedProperty)
			{
				typedProperty->m_data = propertyValue;
				return ePropertySetResult_SUCCESS_UPDATE;
			}

			//Property is not correct type
			else
			{
				delete found->second;
				found->second = new NamedProperty<Type>(propertyValue);
				return ePropertySetResult_SUCCESS_CHANGED_TYPE;
			}
		}
	}
};
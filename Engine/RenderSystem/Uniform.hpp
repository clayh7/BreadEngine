#pragma once

#include <string>
#include <map>


//-------------------------------------------------------------------------------------------------
class Uniform
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	int m_bindPoint;
	int m_length;
	int m_size;
	unsigned int m_type;
	std::string m_name;
	void * const m_data;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	Uniform(Uniform const & copy, void * const data);
	Uniform(int bindPoint, int length, int size, unsigned int type, std::string const & name, void * const data);
	~Uniform();

	int GetBindPoint() const { return m_bindPoint; }
};

// This fancy struct is so I can use strings as keys in a map, and compare their contents instead
// of their pointers.
struct CompareCharPointers
{
	bool operator()(char const *a, char const *b) const
	{
		return std::strcmp(a, b) < 0;
	}
};

typedef std::map<const char*, Uniform*, CompareCharPointers> UniformMap;
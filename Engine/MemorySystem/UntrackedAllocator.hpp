#pragma once

#undef max //Needed for max() function from limits
#include <cstddef>
#include <limits>
#include <memory>

//-------------------------------------------------------------------------------------------------
// Handles static STL containers, because their memory allocations won't be detected by this
// system, and it'll this it's a memory leak. Try to use no static STL containers.
//
// Example Usage
// static std::vector<int, UntrackedAllocator<int>> gTestVector;
//-------------------------------------------------------------------------------------------------
template <typename Type>
class UntrackedAllocator
{
public:
	// Convert an allocator<Type> to allocator<OtherType>
	template<typename OtherType>
	struct rebind
	{
		typedef UntrackedAllocator<OtherType> other;
	};

	//-------------------------------------------------------------------------------------------------
	// Typedefs
	//-------------------------------------------------------------------------------------------------
public:
	typedef Type value_type;
	typedef const Type* const_pointer;
	typedef size_t size_type;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	inline explicit UntrackedAllocator()
	{
		//Nothing
	}

	inline ~UntrackedAllocator()
	{
		//Nothing
	}

	inline explicit UntrackedAllocator(UntrackedAllocator const &)
	{
		//Nothing
	}

	template<typename OtherType>
	inline explicit UntrackedAllocator(UntrackedAllocator<OtherType> const &)
	{
		//Nothing
	}

	inline value_type * address(value_type & r)
	{
		return &r;
	}

	inline value_type const * address(value_type const & r)
	{
		return &r;
	}

	inline value_type * allocate(size_t cnt, typename std::allocator<void>::const_pointer = 0)
	{
		value_type * ptr = (value_type*)malloc(cnt * sizeof(value_type));
		return ptr;
	}

	inline void deallocate(value_type * p, size_t)
	{
		free(p);
	}

	inline size_t max_size() const
	{
		return std::numeric_limits<size_t>::max() / sizeof(value_type);
	}

	inline void construct(value_type * p, value_type const & t)
	{
		new(p) value_type(t);
	}

	inline void destroy(value_type * p)
	{
		p; //It thinks it's unreferenced?
		p->~value_type();
	}

	inline bool operator==(UntrackedAllocator const & a)
	{
		return this == &a;
	}

	inline bool operator!=(UntrackedAllocator const & a)
	{
		return this != &a;
	}
};
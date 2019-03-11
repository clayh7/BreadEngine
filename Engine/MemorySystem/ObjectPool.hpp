#pragma once

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
class BlockNode
{
public:
	BlockNode * next;
};


//-------------------------------------------------------------------------------------------------
template <typename Type>
class ObjectPool
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	Type * m_nextFreePtr;
	Type * m_buffer;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	ObjectPool(size_t blockCount)
	{
		size_t bufferSize = sizeof(Type) * blockCount;
		bool check = sizeof(Type) >= sizeof(BlockNode);
		ASSERT_RECOVERABLE(check, "ObjectPool type not large enough");
		m_buffer = (Type*)malloc(bufferSize);
		m_nextFreePtr = nullptr;
		for(int blockIndex = blockCount - 1; blockIndex >= 0; --blockIndex)
		{
			Type * blockPtr = &m_buffer[blockIndex];
			BlockNode * node = (BlockNode*)blockPtr;
			node->next = (BlockNode*)m_nextFreePtr;
			m_nextFreePtr = (Type*)node;
		}
	}

	Type * Alloc(Type & data)
	{
		Type * objectBlock = Alloc();
		memcpy(&data, objectBlock, sizeof(Type));
		return objectBlock;
	}

	Type * Alloc()
	{
		Type * objectBlock = (Type*)m_nextFreePtr;
		ASSERT_RECOVERABLE(objectBlock != nullptr, "Out of memory in ObjectPool");
		m_nextFreePtr = (Type*)(((BlockNode*)m_nextFreePtr)->next);
		new (objectBlock) Type();
		return objectBlock;
	}

	void Delete(Type * objectBlock)
	{
		objectBlock->~Type();
		BlockNode * freed = (BlockNode*)objectBlock;
		freed->next = (BlockNode*)m_nextFreePtr;
		m_nextFreePtr = (Type*)freed;
	}

	void Destroy()
	{
		free(m_buffer);
	}
};
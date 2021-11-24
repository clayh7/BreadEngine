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
	Type* m_nextFreePtr;
	Type* m_buffer;
	size_t m_usedBlocks;
	size_t m_highBlockCount;
	const char* m_poolName;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	ObjectPool(size_t blockCount, const char* name)
		: m_nextFreePtr(nullptr)
		, m_buffer(nullptr)
		, m_usedBlocks(0)
		, m_highBlockCount(0)
		, m_poolName(name)
	{
		size_t bufferSize = sizeof(Type) * blockCount;
		bool check = sizeof(Type) >= sizeof(BlockNode);
		ASSERT_RECOVERABLE(check, "ObjectPool type not large enough");
		m_buffer = (Type*)malloc(bufferSize);
		for(int blockIndex = blockCount - 1; blockIndex >= 0; --blockIndex)
		{
			Type* blockPtr = &m_buffer[blockIndex];
			BlockNode * node = (BlockNode*)blockPtr;
			node->next = (BlockNode*)m_nextFreePtr;
			m_nextFreePtr = (Type*)node;
		}
	}

	Type* Alloc(Type & data)
	{
		Type* objectBlock = Alloc();
		memcpy(&data, objectBlock, sizeof(Type));
		return objectBlock;
	}

	Type* Alloc()
	{
		m_usedBlocks++;
		if (m_usedBlocks > m_highBlockCount)
		{
			m_highBlockCount = m_usedBlocks;
		}
		Type* objectBlock = (Type*)m_nextFreePtr;
		ASSERT_RECOVERABLE(objectBlock, "Out of memory in ObjectPool");
		m_nextFreePtr = (Type*)(((BlockNode*)m_nextFreePtr)->next);
		new (objectBlock) Type();
		return objectBlock;
	}

	void Delete(Type* objectBlock)
	{
		m_usedBlocks--;
		objectBlock->~Type();
		BlockNode* freed = (BlockNode*)objectBlock;
		freed->next = (BlockNode*)m_nextFreePtr;
		m_nextFreePtr = (Type*)freed;
	}

	void Destroy()
	{
		free(m_buffer);
		DebuggerPrintf("\n//=============================================================================================\n");
		DebuggerPrintf("Object Pool [%s] \n", m_poolName);
		DebuggerPrintf("High Block Count: %u \n", m_highBlockCount);
		DebuggerPrintf("//=============================================================================================\n\n");
	}
};
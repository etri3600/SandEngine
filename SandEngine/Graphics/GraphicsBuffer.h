#pragma once

struct SDescriptorBlock
{
	SDescriptorBlock(unsigned int id)
		: blockID(id)
	{}

	const unsigned int blockID;

	void* pBuffer = nullptr;
	unsigned int size = 0;
	unsigned int offset = ~0u;
};
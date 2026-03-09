#pragma once
#include <inttypes.h>

namespace fb
{
	struct EntityEvent {
		void** vftptr;
		uint32_t eventId;
		bool b_unk;
	};
}
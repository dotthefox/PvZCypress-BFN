#pragma once
#include <MemUtil.h>

#define OFFSET_FB_ENGINEPEER_INIT CYPRESS_GW_SELECT(0x1408038F0, 0x14070CF70, 0x1410FDE90)

namespace Kyber
{
	class SocketManager;
}

DECLARE_HOOK(
	fb_EnginePeer_init,
	__fastcall,
	void,

	void* thisPtr,
	Kyber::SocketManager* socketManager,
	const char* address,
	int titleId,
	int versionId
);
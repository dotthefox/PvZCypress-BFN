#pragma once
#include <MemUtil.h>
#include <fb/Engine/Server.h>
#include <fb/Engine/LevelSetup.h>
#include <fb/Engine/ServerPlayer.h>
#include <fb/Engine/ServerPlayerManager.h>
#include <fb/Engine/ServerConnection.h>
#include <fb/SecureReason.h>
#include <Kyber/SocketManager.h>

#include <fb/Engine/Schematics.h>

#define OFFSET_FB_SERVERPVZROUNDCONTROL_EVENT 0x141E34DB0

#if(HAS_DEDICATED_SERVER)
DECLARE_HOOK(
	fb_Server_start,
	__fastcall,
	__int64,

	void* thisPtr,
	fb::ServerSpawnInfo* info,
	Kyber::ServerSpawnOverrides* spawnOverrides
);

DECLARE_HOOK(
	fb_Server_update,
	__fastcall,
	bool,

	void* thisPtr,
	void* params
);

DECLARE_HOOK(
	fb_windowProcedure,
	__stdcall,
	__int64,

	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

DECLARE_HOOK(
	fb_ServerPVZRoundControl_event,
	__fastcall,
	void,

	void* thisPtr,
	struct fb::EntityEvent* event
);

DECLARE_HOOK(
	fb_ServerConnection_onCreatePlayerMsg,
	__fastcall,
	void*,

	fb::ServerConnection* thisPtr,
	void* msg
);

DECLARE_HOOK(
	fb_ServerPlayerManager_addPlayer,
	__fastcall,
	void*,

	fb::ServerPlayerManager* thisPtr,
	fb::ServerPlayer* player,
	const char* nickname
);

DECLARE_HOOK(
	fb_ServerPlayer_disconnect,
	__fastcall,
	void,

	fb::ServerPlayer* thisPtr,
	fb::SecureReason reason,
	eastl::new_string* reasonText
);

#endif
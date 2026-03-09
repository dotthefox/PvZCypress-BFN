#pragma once
#include <MemUtil.h>
#include <EASTL/new_string.h>
#include <fb/Engine/Client.h>
#include <fb/SecureReason.h>

DECLARE_HOOK(
	fb_Client_enterState,
	__fastcall,
	void,

	void* thisPtr,
	fb::ClientState state,
	fb::ClientState prevState
);

DECLARE_HOOK(
	fb_OnlineManager_connectToAddress,
	__fastcall,
	void,

	void* thisPtr,
	const char* ipAddr,
	const char* serverPassword
);

DECLARE_HOOK(
	fb_OnlineManager_onGotDisconnected,
	__fastcall,
	void,

	void* thisPtr,
	fb::SecureReason reason,
	eastl::new_string* reasonText
);

DECLARE_HOOK(
	fb_EAUser_ctor,
	__fastcall,
	void*,

	void* thisPtr,
	int localPlayerId,
	void* a3,
	void* controller
)
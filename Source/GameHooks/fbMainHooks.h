#pragma once

#include <fb/Main.h>
#include <fb/Engine/ScriptContext.h>

#define OFFSET_ENVIRONMENT_GETHOSTIDENTIFIER CYPRESS_GW_SELECT(0x1403A9170, 0x1401F7C00, 0x14047D6F0)

#define OFFSET_CONSOLE__WRITECONSOLE CYPRESS_GW_SELECT(0x140392720, 0x1401A7850, 0x1404622D0)
#define OFFSET_CONSOLE__ENQUEUECOMMAND CYPRESS_GW_SELECT(0x14038FA50, 0x1401A7680)

#define OFFSET_FB_ISINTERACTIONALLOWED 0x140B8F820
#define OFFSET_FB_TICKERSALLOWEDTOSHOW 0x140E0A8F0

#define OFFSET_BFN_FB_LUAOPTAPPLYSETTINGS 0x140F07F60
#define OFFSET_FB_MAIN_INITSETTINGS 0x142218250

DECLARE_HOOK(
	fb_Main_initSettings,
	__fastcall,
	void,

	void* thisPtr
);

DECLARE_HOOK(
	fb_Environment_getHostIdentifier,
	__cdecl,
	const char*
);

DECLARE_HOOK(
	fb_Console_writeConsole,
	__cdecl,
	void,

	const char* tag,
	const char* buffer,
	unsigned int size
);

DECLARE_HOOK(
	fb_realMain,
	__cdecl,
	__int64,

	HINSTANCE hIntance,
	HINSTANCE hPrevInstance,
	const char* lpCmdLine,
	__int64 a4,
	__int64 a5
);

DECLARE_HOOK(
	fb_main_createWindow,
	__fastcall,
	void,

	void* thisPtr
);

DECLARE_HOOK(
	fb_luaApplySettings,
	__cdecl,
	void,

	uintptr_t luaOptionsManager,
	fb::ScriptContext* scontext,
	bool inmediate
);
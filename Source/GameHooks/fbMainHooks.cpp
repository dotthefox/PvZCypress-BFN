#include "pch.h"
#include <sstream>

#include "fbMainHooks.h"
#include <Core/Program.h>
#include <fb/Engine/ExecutionContext.h>

DEFINE_HOOK(
	fb_Main_initSettings,
	__fastcall,
	void,

	void* thisPtr
)
{
	auto& options = *reinterpret_cast<eastl::vector<const char*>*>(OFFSET_ECDATA_START);

	std::stringstream cmdLine;
	cmdLine << "commandLine = {";

	if (!options.empty())
	{
		for (int o = 0; o < options.size(); o++)
		{
			const auto option = options.at(o);

			if (*option == '-')
			{
				std::string l_option = option + 1;
				const char* value = nullptr;

				std::transform(l_option.begin(), l_option.end(), l_option.begin(), tolower);

				cmdLine << "[ [==[" << l_option << "]==] ] = ";

				if (o + 1 < options.size())
				{
					value = options.at(o + 1);

					if (*value == '-')
						value = nullptr;
				}

				if (value)
					cmdLine << "[==[" << value << "]==], ";
				else
					cmdLine << "true, ";
			}

			cmdLine << "[" << o << "] = [==[" << option << "]==], ";
		}
	}

	cmdLine << "}";
	fb::ScriptContext::context()->executeString(cmdLine.str().c_str());

	CYPRESS_LOGMESSAGE(LogLevel::Info, "Applied commandline");

	Orig_fb_Main_initSettings(thisPtr);
}

DEFINE_HOOK(
	fb_Environment_getHostIdentifier,
	__cdecl,
	const char*
)
{
	if (!g_program->IsServer())
	{
		return g_program->GetClient()->GetPlayerName();
	}
	return Orig_fb_Environment_getHostIdentifier();
}

DEFINE_HOOK(
	fb_Console_writeConsole,
	__cdecl,
	void,

	const char* tag,
	const char* buffer,
	unsigned int size
)
{
	// we only care about messages tagged with ingame
	if (g_program->IsServer() && strcmp(tag, "ingame") == 0)
	{
		std::string_view msgView(buffer, size);
		CYPRESS_LOGTOSERVER(LogLevel::Info, "{}", msgView);
	}
	Orig_fb_Console_writeConsole(tag, buffer, size);
}

DEFINE_HOOK(
	fb_realMain,
	__cdecl,
	__int64,

	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	const char* lpCmdLine,
	__int64 a4,
	__int64 a5
)
{
	g_program->InitConfig();

	CYPRESS_LOGMESSAGE(LogLevel::Info, "Initializing {} (Cypress version {})", CYPRESS_GAME_NAME, GetCypressVersion().c_str());
	return Orig_fb_realMain(hInstance, hPrevInstance, lpCmdLine, a4, a5);
}

DEFINE_HOOK(
	fb_main_createWindow,
	__fastcall,
	void,

	void* thisPtr
)
{
	if (g_program->IsServer())
	{
		HBRUSH brush = GetSysColorBrush(COLOR_WINDOW);
		ptrset(thisPtr, 0x170, brush);
	}

	Orig_fb_main_createWindow(thisPtr);

	if (g_program->IsServer())
	{
		g_program->GetServer()->InitThinClientWindow();
	}
}
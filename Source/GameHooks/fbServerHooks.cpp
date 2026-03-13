#include "pch.h"
#include "fbServerHooks.h"

#include <string>
#include <Core/Program.h>
#include <Core/Console/ConsoleFunctions.h>
#include <fb/Engine/Server.h>
#include <fb/Engine/ServerGameContext.h>

#include <fb/Engine/ScriptContext.h>
#include <fb/SecureReason.h>

#if(HAS_DEDICATED_SERVER)
DEFINE_HOOK(
	fb_Server_start,
	__fastcall,
	__int64,

	void* thisPtr,
	fb::ServerSpawnInfo* info,
	Kyber::ServerSpawnOverrides* spawnOverrides
)
{
#if(HAS_DEDICATED_SERVER)
	if (!g_program->GetInitialized())
	{
		bool wsaInit = g_program->InitWSA();
		CYPRESS_ASSERT(wsaInit, "WSA failed to initialize!");
		g_program->SetInitialized(true);
	}
#endif
	return Orig_fb_Server_start(thisPtr, info, spawnOverrides);
}

DEFINE_HOOK(
	fb_Server_update,
	__fastcall,
	bool,

	void* thisPtr,
	void* params
)
{
#if(HAS_DEDICATED_SERVER)
	bool updated = Orig_fb_Server_update(thisPtr, params);
	if (g_program->IsServer())
	{
		Cypress::Server* server = g_program->GetServer();

		server->UpdateStatus(thisPtr, (float)ptrread<int>(params, 0x18) * 0.000000001);
		
		bool statusUpdated = !server->GetStatusUpdated();
		server->SetStatusUpdated(true);
		if (statusUpdated)
		{
			PostMessageA(*server->GetMainWindow(), WM_APP_UPDATESTATUS, 0, 0);
		}
	}
	return updated;
#else
	Orig_fb_Server_update(thisPtr, params);
#endif
}

DEFINE_HOOK(
	fb_windowProcedure,
	__stdcall,
	__int64,

	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (g_program->IsServer())
	{
		Cypress::Server* pServer = g_program->GetServer();
		HWND commandBox = pServer->GetCommandBox();

		switch (msg)
		{
		case WM_SIZE:
		{
			if (commandBox != NULL)
			{
				RECT rect;
				GetClientRect(*pServer->GetMainWindow(), &rect);

				MoveWindow(pServer->GetListBox(), 0, 0, rect.right, rect.bottom - 88, 1);
				MoveWindow(commandBox, 0, rect.bottom - 88, rect.right, 17, 1);
				MoveWindow(pServer->GetToggleLogButtonBox(), rect.right - 80, rect.bottom - 16, 80, 16, 1);

				int index = 0, width = 0;
				HWND* currentBox = pServer->GetStatusBox();

				do
				{
					if (index == 4)
						width = rect.right - 800;
					else
						width = 200;
					MoveWindow(*currentBox++, 200 * index++, rect.bottom - 71, width, 71, 1);

				} while (index < 5);
			}
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
		default: break;
		}

		if (msg == WM_APP_UPDATESTATUS && commandBox)
		{
			HWND* g_statusBox = pServer->GetStatusBox();
			SetWindowTextA(*g_statusBox, pServer->GetStatusColumn1().c_str());
			SetWindowTextA(*++g_statusBox, pServer->GetStatusColumn2().c_str());
			return DefWindowProcA(hWnd, msg, wParam, lParam);
		}
		else if (msg == WM_COMMAND)
		{
			if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == pServer->GetToggleLogButtonBox())
			{
				if (!pServer->GetServerLogEnabled())
				{
					pServer->SetServerLogEnabled(true);
					CYPRESS_LOGTOSERVER(LogLevel::Info, "Log window enabled.");
					SetWindowTextA(pServer->GetToggleLogButtonBox(), "Disable Logs");
					PostMessageA(*pServer->GetMainWindow(), WM_APP_UPDATESTRINGLIST, 0, 0);
				}
				else
				{
					CYPRESS_LOGTOSERVER(LogLevel::Info, "Log window disabled.");
					pServer->SetServerLogEnabled(false);
					SetWindowTextA(pServer->GetToggleLogButtonBox(), "Enable Logs");
				}
				return DefWindowProcA(hWnd, msg, wParam, lParam);
			}
		}
	}
	return Orig_fb_windowProcedure(hWnd, msg, wParam, lParam);
}

DEFINE_HOOK(
	fb_ServerPVZRoundControl_event,
	__fastcall,
	void,

	void* thisPtr,
	fb::EntityEvent* event
)
{
	Orig_fb_ServerPVZRoundControl_event(thisPtr, event);

	if (event->eventId == 0xDB4B330) //Reset
	{
		Cypress::Server* pServer = g_program->GetServer();

		if (pServer->IsUsingPlaylist())
		{
			fb::LevelSetup nextLevelSetup;

			const auto nextSetup = pServer->GetServerPlaylist()->GetNextSetup();
			pServer->LevelSetupFromPlaylistSetup(&nextLevelSetup, nextSetup);
			pServer->ApplySettingsFromPlaylistSetup(nextSetup);

			fb::PostServerLoadLevelMessage(&nextLevelSetup, true, false);
		}
	}
}

DEFINE_HOOK(
	fb_ServerConnection_onCreatePlayerMsg,
	__fastcall,
	void*,

	fb::ServerConnection* thisPtr,
	void* msg
)
{
	const char* playerName = reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(msg) + 0x6C);

	if (g_program->GetServer()->GetServerBanlist()->IsBanned(playerName, thisPtr->m_machineId.c_str()))
	{
		thisPtr->disconnect(fb::SecureReason_Banned, "Banned from server");
	}

	int nameLen = strlen(playerName);
	if (nameLen < 3 || nameLen > 16)
	{
		thisPtr->disconnect(fb::SecureReason_KickedOut, "Invalid Username Length");
	}

	for (const char* p = playerName; *p != '\0'; ++p)
	{
		if (iscntrl(static_cast<unsigned char>(*p)))
		{
			thisPtr->disconnect(fb::SecureReason_KickedOut, "Invalid Characters in Username");
			break;
		}
	}
	// If you wish to remove these characters from your build of the server, remove the start quotation, where the start is #
	static const char* bannedChars = "\t\n!\"#$%&'()*+,./:;<=>?@[\\]^`{|}~";
	if (strpbrk(playerName, bannedChars) != nullptr)
	{
		thisPtr->disconnect(fb::SecureReason_KickedOut, "Invalid Characters in Username");
	}

	CYPRESS_LOGTOSERVER(LogLevel::Info, "{} is trying to join from machine {}", playerName, thisPtr->m_machineId.c_str());
	return Orig_fb_ServerConnection_onCreatePlayerMsg(thisPtr, msg);
}

DEFINE_HOOK(
	fb_ServerPlayerManager_addPlayer,
	__fastcall,
	void*,

	fb::ServerPlayerManager* thisPtr,
	fb::ServerPlayer* player,
	const char* nickname
)
{
	if (!player->isAIPlayer())
	{
		CYPRESS_LOGTOSERVER(LogLevel::Info, "[Id: {}] {} has joined the server", player->getPlayerId(), nickname);
	}
	return Orig_fb_ServerPlayerManager_addPlayer(thisPtr, player, nickname);
}


DEFINE_HOOK(
	fb_ServerPlayer_disconnect,
	__fastcall,
	void,

	fb::ServerPlayer* thisPtr,
	fb::SecureReason reason,
	eastl::new_string* reasonText
)
{
	const char* reasonStr = "None provided";

	if (!reasonText->empty())
		reasonStr = reasonText->c_str();

	CYPRESS_LOGTOSERVER(LogLevel::Info, "[Id: {}] {} has left the server (Reason: {}, {})",
		thisPtr->getPlayerId(),
		thisPtr->m_name,
		reasonStr,
		fb::SecureReason_ToString(reason));

	Orig_fb_ServerPlayer_disconnect(thisPtr, reason, reasonText);
}
#endif
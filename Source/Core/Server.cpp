#include "pch.h"
#include "Psapi.h"
#include "Server.h"
#include <string>
#include <sstream>
#include <Core/Program.h>
#include <Core/Settings.h>
#include <fb/Engine/LevelSetup.h>
#include <fb/Engine/Server.h>
#include <fb/Main.h>
#include <fb/Engine/String.h>
#include <fb/Engine/ServerPlayerManager.h>
#include <fb/Engine/ServerPeer.h>
#include <fb/Engine/ExecutionContext.h>
#include <Core/Console/ConsoleFunctions.h>
#include <fb/Engine/ServerGameContext.h>

#include <fb/Engine/Console.h>

using namespace fb;

#if(HAS_DEDICATED_SERVER)
namespace Cypress
{
	WNDPROC editBoxWndProc;
	LRESULT CALLBACK EditBoxWndProcProxy(HWND hWnd, uint32_t msg, uint32_t wParam, LPARAM lParam)
	{
		if (msg == WM_KEYDOWN)
		{
			if (wParam == VK_RETURN)
			{
				char buffer[1024] = { 0 };
				GetWindowTextA(g_program->GetServer()->GetCommandBox(), buffer, sizeof(buffer));
				SetWindowTextA(g_program->GetServer()->GetCommandBox(), "");

				CYPRESS_LOGTOSERVER(LogLevel::Info, "{}", buffer);

				if (!Cypress::HandleCommand(std::string(buffer)))
				{
					fb::Console::enqueueCommand(std::format("ingame|{}", buffer).c_str());				
				}
			}
		}
		return editBoxWndProc(hWnd, msg, wParam, lParam);
	}

	void Server::InitThinClientWindow()
	{
		m_mainWindow = (HWND*)0x14421BA88;

		HINSTANCE hInstance = GetModuleHandleA(nullptr);
		WPARAM fontObj = (WPARAM)GetStockObject(ANSI_VAR_FONT);

		SetWindowPos(*m_mainWindow, NULL, 0, 0, 1024, 720, NULL);

		RECT rect;
		GetClientRect(*m_mainWindow, &rect);

		m_listBox = CreateWindowExA(NULL, "LISTBOX", nullptr, 0x50201000, 0, 0, rect.right, rect.bottom - 88, *m_mainWindow, NULL, hInstance, nullptr);
		SendMessageA(m_listBox, WM_SETFONT, fontObj, NULL);

		m_commandBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", nullptr, WS_CHILD | WS_VISIBLE, 0, rect.bottom - 88, rect.right, 17, *m_mainWindow, NULL, hInstance, nullptr);
		SendMessageA(m_commandBox, WM_SETFONT, fontObj, NULL);

		m_toggleLogButtonBox = CreateWindowExA(NULL, "BUTTON", nullptr, 0x54000000, rect.right - 80, rect.bottom - 16, 80, 16, *m_mainWindow, NULL, hInstance, nullptr);
		SendMessageA(m_toggleLogButtonBox, WM_SETFONT, fontObj, NULL);
		SetWindowTextA(m_toggleLogButtonBox, m_serverLogEnabled ? "Disable Logs" : "Enable Logs");

		int x = 0, current = 0;
		HWND* currentBox = m_statusBox;

		do
		{
			current = x + 200;
			*currentBox = CreateWindowExA(NULL, "EDIT", nullptr, 0x54000804, x, rect.bottom - 71, 200, 71, *m_mainWindow, NULL, hInstance, NULL);
			SendMessageA(*currentBox, WM_SETFONT, fontObj, NULL);
			currentBox++;
			x = current;

		} while (current < 1000);

		editBoxWndProc = (WNDPROC)SetWindowLongPtrA(m_commandBox, GWLP_WNDPROC, (LONG_PTR)EditBoxWndProcProxy);
		
		SetWindowTextW(*m_mainWindow, L"PVZ Battle for Neighborville Server");
		UpdateWindow(*m_mainWindow);
	}

	void Server::ServerRestartLevel(ArgList args)
	{
		void* fbServer = g_program->GetServer()->GetFbServerInstance();
		if (!fbServer) return;

		void* curLevel = ptrread<void*>(fbServer, CYPRESS_GW_SELECT(0xA0, 0xF0, 0xC8));
		if (!curLevel) return;

		fb::LevelSetup* setup = (fb::LevelSetup*)((__int64)curLevel + CYPRESS_GW_SELECT(0x40, 0x118, 0x118));

		fb::PostServerLoadLevelMessage(setup, true, false);
	}

	void Server::ServerLoadLevel(ArgList args)
	{
		int numArgs = args.size();

		if (numArgs < 3) return;

		std::string& levelName = args[0];
		std::string& inclusion = args[1];
		std::string& startpoint = args[2];

		LevelSetup setup;
		if (strstr(levelName.c_str(), "Levels/") == 0)
		{
			setup.m_levelManagerInitialLevel = std::format("Levels/{}/{}", levelName.c_str(), levelName.c_str());
		}
		else
		{
			setup.m_levelManagerInitialLevel = levelName.c_str();
		}
		setup.setInclusionOptions(inclusion.c_str());

		setup.m_name = "Levels/Level_Picnic_Root/Level_Picnic_Root";
		setup.m_levelManagerStartPoint = startpoint.c_str();

		switch (numArgs)
		{
		case 4:
			setup.m_loadScreen_GameMode = args[2].c_str();
			break;
		case 5:
			setup.m_loadScreen_GameMode = args[2].c_str();
			setup.m_loadScreen_LevelName = args[3].c_str();
			break;
		case 6:
			setup.m_loadScreen_GameMode = args[2].c_str();
			setup.m_loadScreen_LevelName = args[3].c_str();
			setup.m_loadScreen_LevelDescription = args[4].c_str();
			break;
		}

		fb::PostServerLoadLevelMessage(&setup, true, false);
	}

	void Server::ServerLoadNextRound(ArgList args)
	{
		if (!g_program->GetServer()->IsUsingPlaylist())
		{
			CYPRESS_LOGTOSERVER(LogLevel::Info, "Server is not using a playlist.");
			return;
		}

		const PlaylistLevelSetup* nextSetup = g_program->GetServer()->GetServerPlaylist()->GetNextSetup();
		g_program->GetServer()->LoadPlaylistSetup(nextSetup);
	}

	void Server::ServerKickPlayer(ArgList args)
	{
		ServerGameContext* gameContext = ServerGameContext::GetInstance();
		if (!gameContext) return;
		if (!gameContext->m_serverPlayerManager) return;

		ServerPlayer* player = gameContext->m_serverPlayerManager->findHumanByName(args[0].c_str());
		if (!player)
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} not found!", args[0].c_str());
			return;
		}
		if (player->isAIOrPersistentAIPlayer())
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} is an AI!", args[0].c_str());
			return;
		}

		ServerConnection* connection = gameContext->m_serverPeer->connectionForPlayer(player);
		if (!connection) return;

		const char* reason = "Kicked by Admin";

		if (args.size() > 1)
		{
			reason = args[1].c_str();
		}

		CYPRESS_LOGTOSERVER(LogLevel::Info, "Kicked {} ({})", player->m_name, reason);
		connection->disconnect(SecureReason_KickedOut, reason);
	}

	void Server::ServerKickPlayerById(ArgList args)
	{
		ServerGameContext* gameContext = ServerGameContext::GetInstance();
		if (!gameContext) return;
		if (!gameContext->m_serverPlayerManager) return;

		ServerPlayer* player = gameContext->m_serverPlayerManager->getById(std::atoi(args[0].c_str()));
		if (!player)
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} not found!", args[0].c_str());
			return;
		}
		if (player->isAIOrPersistentAIPlayer())
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} is an AI!", args[0].c_str());
			return;
		}

		ServerConnection* connection = gameContext->m_serverPeer->connectionForPlayer(player);
		if (!connection) return;

		const char* reason = "Kicked by Admin";

		if (args.size() > 1)
		{
			reason = args[1].c_str();
		}

		CYPRESS_LOGTOSERVER(LogLevel::Info, "Kicked {} ({})", player->m_name, reason);
		connection->disconnect(SecureReason_KickedOut, reason);
	}

	void Server::ServerBanPlayer(ArgList args)
	{
		ServerGameContext* gameContext = ServerGameContext::GetInstance();
		if (!gameContext) return;
		if (!gameContext->m_serverPlayerManager) return;

		ServerPlayer* player = gameContext->m_serverPlayerManager->findHumanByName(args[0].c_str());
		if (!player)
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} not found!", args[0].c_str());
			return;
		}
		if (player->isAIOrPersistentAIPlayer())
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} is an AI!", args[0].c_str());
			return;
		}

		ServerConnection* connection = gameContext->m_serverPeer->connectionForPlayer(player);
		if (!connection) return;

		const char* reasonText = "The Ban Hammer has spoken!";

		if (args.size() > 1)
		{
			reasonText = args[1].c_str();
		}

		g_program->GetServer()->GetServerBanlist()->AddToList(player->m_name, connection->m_machineId.c_str(), reasonText);
		connection->disconnect(SecureReason_Banned, reasonText);
	}

	void Server::ServerBanPlayerById(ArgList args)
	{
		ServerGameContext* gameContext = ServerGameContext::GetInstance();
		if (!gameContext) return;
		if (!gameContext->m_serverPlayerManager) return;

		ServerPlayer* player = gameContext->m_serverPlayerManager->getById(std::atoi(args[0].c_str()));
		if (!player)
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} not found!", args[0].c_str());
			return;
		}
		if (player->isAIOrPersistentAIPlayer())
		{
			CYPRESS_LOGTOSERVER(LogLevel::Error, "Player {} is an AI!", args[0].c_str());
			return;
		}

		ServerConnection* connection = gameContext->m_serverPeer->connectionForPlayer(player);
		if (!connection) return;

		const char* reasonText = "Banned by server admin";

		if (args.size() > 1)
		{
			reasonText = args[1].c_str();
		}

		g_program->GetServer()->GetServerBanlist()->AddToList(player->m_name, connection->m_machineId.c_str(), reasonText);
		connection->disconnect(SecureReason_Banned, reasonText);
	}

	void Server::ServerUnbanPlayer(ArgList args)
	{
		ServerGameContext* gameContext = ServerGameContext::GetInstance();
		if (!gameContext) return;

		const char* playerName = args[0].c_str();
		if (!g_program->GetServer()->GetServerBanlist()->IsBanned(playerName))
		{
			CYPRESS_LOGTOSERVER(LogLevel::Info, "Player {} is not banned", playerName);
			return;
		}

		Server* pServer = g_program->GetServer();

		auto& entry = pServer->GetServerBanlist()->GetPlayerEntry(playerName);

		pServer->GetServerBanlist()->RemoveFromList(playerName, entry.MachineId.c_str());

		CYPRESS_LOGTOSERVER(LogLevel::Info, "Player {} has been unbanned", playerName);
	}

	Server::Server()
		: m_socketManager(new Kyber::SocketManager(Kyber::ProtocolDirection::Clientbound, Kyber::SocketSpawnInfo(false, "", "")))
		, m_fbServerInstance(nullptr)
		, m_mainWindow(nullptr)
		, m_listBox(NULL)
		, m_commandBox(NULL)
		, m_toggleLogButtonBox(NULL)
		, m_running(false)
		, m_statusUpdated(false)
		, m_serverLogEnabled(false)
		, m_usingPlaylist(false)
		, m_statusCol1()
		, m_statusCol2()
		, m_banlist()
		, m_playlist()
		, m_statusBox{NULL, NULL, NULL, NULL, NULL}
	{
	}

	Server::~Server()
	{
	}

	void Server::UpdateStatus(void* fbServerInstance, float deltaTime)
	{
		static unsigned int startSystemTime = GetSystemTime();
		unsigned int sec = (GetSystemTime() - startSystemTime) / 1000;
		unsigned int min = (sec / 60) % 60;
		unsigned int hour = (sec / (60 * 60));

		static unsigned int lastSec = 0;
		static float currentDeltaTime = 0;
		static float sumDeltaTime = 0;
		static unsigned int frameCount = 0;

		m_fbServerInstance = fbServerInstance;

		sumDeltaTime += deltaTime;
		++frameCount;

		if (lastSec != sec)
		{
			lastSec = sec;
			currentDeltaTime = sumDeltaTime / float(frameCount);
			sumDeltaTime = 0;
			frameCount = 0;
		}

		fb::ServerPlayerManager* playerMgr = ptrread<fb::ServerPlayerManager*>(fbServerInstance, 0xB0);
		fb::ServerPeer* serverPeer = ptrread<fb::ServerPeer*>(fbServerInstance, 0xD8);
		void* ghostMgr = serverPeer->GetGhostManager();

		if (ghostMgr)
			ghostMgr = *(void**)((*(__int64*)((uintptr_t)ghostMgr + 0x8)) + 0x30);
		
		unsigned int numGhosts = ghostMgr ? ptrread<unsigned int>(ghostMgr, 0x190) : 0;
		unsigned int maxPlayerCount = *(int*)(*(__int64*)0x143FEAB80 + 0x44);

		if (serverPeer)
			maxPlayerCount = std::min(maxPlayerCount, serverPeer->maxClientCount());

		std::string playerCountStr = std::format("{}/{} ({}/{}) [{}]",
			playerMgr->humanPlayerCount(),
			maxPlayerCount - playerMgr->maxSpectatorCount(),
			playerMgr->spectatorCount(),
			playerMgr->maxSpectatorCount(),
			maxPlayerCount);

		g_program->GetServer()->SetStatusColumn1(
			std::format(
			"FPS: {} \t\t\t\t"
			"UpTime: {}:{}:{} \t\t\t"
			"PlayerCount: {} \t\t\t"
			"GhostCount: {} \t\t\t"
			"Memory (CPU): {} MB",
			int(1.0f / currentDeltaTime),
			hour,
			min,
			sec % 60,
			playerCountStr,
			numGhosts,
			GetMemoryUsage()
		));

		void* curLevel = ptrread<void*>(fbServerInstance, 0xC8);
		if (curLevel)
		{
			fb::LevelSetup* setup = (fb::LevelSetup*)((__int64)curLevel + 0x118);
			g_program->GetServer()->SetStatusColumn2(
				std::format(
					"Level: {} \t\t"
					"DSub: {} \t\t"
					"GameMode: {} \t\t"
					"StartPoint: {} \t\t"
					"Platform: {}",
					extractFileName(setup->m_name.c_str()),
					setup->m_levelManagerInitialLevel.empty() ? "Not set" : extractFileName(setup->m_levelManagerInitialLevel.c_str()),
					setup->getInclusionOption("GameMode"),
					setup->m_levelManagerStartPoint.empty() ? "Not set" : setup->m_levelManagerStartPoint.c_str(),
					"Win32"
				));
		}
		else
		{
			g_program->GetServer()->SetStatusColumn2("Level: No level");
		}

		static size_t tick = 0;
		size_t fps = int(1.0f / currentDeltaTime);
		if (tick != fps)
		{
			tick = fps;
			g_program->GetServer()->SetStatusUpdated(false);
		}
	}

	size_t Server::GetMemoryUsage()
	{
		PROCESS_MEMORY_COUNTERS_EX pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
			return pmc.WorkingSetSize / (1024 * 1024); //MegaBytes
		}
		return 0;
	}

	void Server::LoadPlaylistSetup(const PlaylistLevelSetup* nextSetup)
	{
		LevelSetup setup;
		LevelSetupFromPlaylistSetup(&setup, nextSetup);
		ApplySettingsFromPlaylistSetup(nextSetup);

		CYPRESS_LOGTOSERVER(LogLevel::Info, "Server is loading playlist setup ({} on {})", setup.m_levelManagerInitialLevel.c_str(), setup.m_levelManagerStartPoint.c_str());
		fb::PostServerLoadLevelMessage(&setup, true, false);
	}

	void Server::LevelSetupFromPlaylistSetup(LevelSetup* setup, const PlaylistLevelSetup* playlistSetup)
	{
		setup->m_name = "Levels/Level_Picnic_Root/Level_Picnic_Root";
		setup->m_levelManagerInitialLevel = playlistSetup->LevelName.c_str();
		setup->m_levelManagerStartPoint = playlistSetup->StartPoint.c_str();

		setup->setInlusionOption("GameMode", playlistSetup->GameMode.c_str());

		if (!playlistSetup->Loadscreen_GamemodeName.empty())
			setup->m_loadScreen_GameMode = playlistSetup->Loadscreen_GamemodeName.c_str();
		if (!playlistSetup->Loadscreen_LevelName.empty())
			setup->m_loadScreen_LevelName = playlistSetup->Loadscreen_LevelName.c_str();
		if (!playlistSetup->Loadscreen_LevelDescription.empty())
			setup->m_loadScreen_LevelDescription = playlistSetup->Loadscreen_LevelDescription.c_str();
	}

	void Server::ApplySettingsFromPlaylistSetup(const PlaylistLevelSetup* playlistSetup)
	{
		if (!playlistSetup->SettingsToApply.empty())
		{
			std::vector<std::string> settings = splitString(playlistSetup->SettingsToApply, '|');

			for (const auto& setting : settings)
			{
				std::vector<std::string> settingAndValue = splitString(setting, ' ');
				if (settingAndValue.size() != 2) continue;

				CYPRESS_LOGTOSERVER(LogLevel::Info, "Playlist is setting {} to {}", settingAndValue[0].c_str(), settingAndValue[1].c_str());
				if (settingAndValue[1].c_str()[0] == '^') // empty string
				{
					fb::SettingsManager::GetInstance()->set(settingAndValue[0].c_str(), "");
				}
				else
				{
					fb::SettingsManager::GetInstance()->set(settingAndValue[0].c_str(), settingAndValue[1].c_str());
				}
			}
		}
	}

	void Server::InitDedicatedServer(void* thisPtr)
	{
		Server* pServer = g_program->GetServer();
		pServer->SetRunning(true);

		LevelSetup initialLevelSetup;

		if (pServer->m_usingPlaylist)
		{
			CYPRESS_LOGTOSERVER(LogLevel::Info, "Loading first setup in playlist");
			const PlaylistLevelSetup* playlistSetup;

			if (pServer->m_playlist.IsMixedMode())
			{
				playlistSetup = pServer->m_playlist.GetMixedLevelSetup();
			}
			else
			{
				pServer->m_playlist.SetCurrentSetup(0);
				playlistSetup = pServer->m_playlist.GetCurrentSetup();
			}

			pServer->LevelSetupFromPlaylistSetup(&initialLevelSetup, playlistSetup);
			pServer->ApplySettingsFromPlaylistSetup(playlistSetup);
		}
		else
		{
			const char* initialLevel = fb::ExecutionContext::getOptionValue("dsub");
			const char* initialInclusion = fb::ExecutionContext::getOptionValue("inclusion");
			const char* startPoint = fb::ExecutionContext::getOptionValue("startpoint");

			CYPRESS_ASSERT(initialLevel != nullptr, "Must provide a DSub name via the -dsub argument!");
			CYPRESS_ASSERT(initialInclusion != nullptr, "Must provide inclusion options via the -inclusion argument!");
			CYPRESS_ASSERT(startPoint != nullptr, "Must provide a startpoint via -startpoint argument!");

			initialLevelSetup.m_name = "Levels/Level_Picnic_Root/Level_Picnic_Root";
			initialLevelSetup.m_levelManagerStartPoint = startPoint;

			if (strstr(initialLevel, "Levels/") == 0)
			{
				initialLevelSetup.m_levelManagerInitialLevel.set(std::format("Levels/{}/{}", initialLevel, initialLevel).c_str());
			}
			else
			{
				initialLevelSetup.m_levelManagerInitialLevel = initialLevel;
			}

			initialLevelSetup.setInclusionOptions(initialInclusion);
		}

		ServerSpawnInfo* spawnInfo = new ServerSpawnInfo(&initialLevelSetup);

		auto fb_spawnServer = reinterpret_cast<void (*)(void* thisPtr, ServerSpawnInfo* info)>(OFFSET_FB_MAIN_SPAWNSERVER);
		fb_spawnServer(reinterpret_cast<void*>(reinterpret_cast<uint64_t>(thisPtr) + 0x8), spawnInfo);

		g_program->GetGameModule()->RegisterCommands();
		pServer->m_banlist.LoadFromFile("bans.json");

		CYPRESS_LOGMESSAGE(LogLevel::Info, "Initialized Dedicated Server");
	}

	unsigned int Server::GetSystemTime()
	{
		static bool l_isInitialized = false;
		static LARGE_INTEGER    l_liPerformanceFrequency;
		static LARGE_INTEGER    l_liBaseTime;

		LARGE_INTEGER liEndTime;

		if (!l_isInitialized) {
			QueryPerformanceFrequency(&l_liPerformanceFrequency);
			QueryPerformanceCounter(&l_liBaseTime);
			l_isInitialized = true;

			return 0;
		}

		unsigned __int64 n;
		QueryPerformanceCounter(&liEndTime);
		n = liEndTime.QuadPart - l_liBaseTime.QuadPart;
		n = n * 1000 / l_liPerformanceFrequency.QuadPart;

		return (unsigned int)n;
	}
}
#endif
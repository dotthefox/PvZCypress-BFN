#pragma once
#include <EASTL/vector.h>
#include <fb/Engine/ServerPlayer.h>

#define OFFSET_SERVERPLAYERMANAGER_ADDPLAYER CYPRESS_GW_SELECT(0x14075B800, 0x140616DC0, 0x140F55A00)

namespace fb
{
    class ServerPlayerManager
    {
    public:
        char pad[0x70];
        eastl::vector<ServerPlayer*> m_players;
        char pad2[0x208];
        eastl::vector<ServerPlayer*> m_spectators;
        char pad3[0x208];
        eastl::vector<ServerPlayer*> m_localPlayers;
        char pad4[0x218];
        ServerPlayer** m_idToPlayerMap;

        int playerCount()
        {
            return m_players.size();
        }

        int spectatorCount()
        {
            return m_spectators.size();
        }

        unsigned int maxSpectatorCount()
        {
            return ptrread<unsigned int>(this, 0x6E8);
        }

        int humanPlayerCount()
        {
            int count = 0;
            for (const auto& player : m_players)
            {
                if (!player->isAIOrPersistentAIPlayer())
                {
                    ++count;
                }
            }
            return count;
        }

        int aiPlayerCount()
        {
            int count = 0;
            for (const auto& player : m_players)
            {
                if (player->isAIOrPersistentAIPlayer())
                {
                    ++count;
                }
            }
            return count;
        }

        ServerPlayer* findByName(const char* name)
        {
            for (size_t i = 0; i < m_players.size(); i++)
            {
                ServerPlayer* player = m_players.at(i);
                if (player && player->m_name && (strcmp(player->m_name, name) == 0))
                {
                    return player;
                }
            }
            return nullptr;
        }

        ServerPlayer* findHumanByName(const char* name)
        {
            int numPlayers = m_players.size();
            for (size_t i = 0; i < numPlayers; i++)
            {
                ServerPlayer* player = m_players.at(i);
                if (player->isAIOrPersistentAIPlayer())
                    continue;

                if (player && player->m_name && (strcmp(player->m_name, name) == 0))
                {
                    return player;
                }
            }
            return nullptr;
        }

        ServerPlayer* getById(unsigned int id)
        {
            if (id < 0 || id > 69)
            {
                return nullptr;
            }
            return m_idToPlayerMap[id];
        }
    }; //Size: 0x10A0
}
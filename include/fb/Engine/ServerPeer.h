#pragma once
#include <EASTL/new_string.h>
#include <EASTL/vector.h>
#include <fb/Engine/ServerPlayer.h>
#include <fb/Engine/ServerConnection.h>

namespace fb
{
    class ServerPeer
    {
    public:

        ServerConnection* connectionForPlayer(ServerPlayer* player)
        {
            auto func = reinterpret_cast<ServerConnection*(*)(ServerPeer*, ServerPlayer*)>(CYPRESS_GW_SELECT(0x140703000, 0x140627760, 0x140F48140));
            return func(this, player);
        }

        void sendMessage(void* networkableMessage, void* filter)
        {
            auto func = reinterpret_cast<void(*)(ServerPeer*, void*, void*)>(0x140627D40);
            func(this, networkableMessage, filter);
        }

        void* GetGhostManager()
        {
            return ptrread<void*>(this, CYPRESS_GW_SELECT(0x3F90, 0x3F90, 0x45F8));
        }

        unsigned int maxClientCount()
        {
            return ptrread<unsigned int>(this, CYPRESS_GW_SELECT(0x3B98, 0x4020, 0x4688));
        }

    };
}
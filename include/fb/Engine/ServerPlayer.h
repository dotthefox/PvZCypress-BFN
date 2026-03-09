#pragma once
#include <cstdint>
#include <MemUtil.h>
#include <fb/SecureReason.h>
#include <EASTL/new_string.h>

#define OFFSET_SERVERPLAYER_DISCONNECT CYPRESS_GW_SELECT(0x14075D860, 0x140614B60, 0x140F57070)

namespace fb
{
    class ServerPlayer
    {
    public:
        char pad_0000[240];
        const char m_name[17];

        unsigned int getPlayerId()
        {
            return ptrread<unsigned int>(this, CYPRESS_GW_SELECT(0xD90, 0x1490, 0x1B8));
        }

        bool isAIPlayer()
        {
            return ptrread<bool>(this, 0x48);
        }

        bool isSpectator()
        {
            return false;
        }

        bool isPersistentAIPlayer()
        {
            return isAIPlayer();
        }

        bool isAIOrPersistentAIPlayer()
        {
            return isAIPlayer() || isPersistentAIPlayer();
        }

        void disconnect(fb::SecureReason reason, eastl::new_string* reasonText)
        {
            auto ServerPlayer__disconnect = reinterpret_cast<void (*)(void* inst, fb::SecureReason reason, eastl::new_string* reasonText)>(OFFSET_SERVERPLAYER_DISCONNECT);
            ServerPlayer__disconnect(this, reason, reasonText);
        }
    };
}
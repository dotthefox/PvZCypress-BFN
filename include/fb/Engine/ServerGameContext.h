#pragma once
#include "ServerPlayerManager.h"
#include "ServerPeer.h"

#define OFFSET_SERVERGAMECONTEXT CYPRESS_GW_SELECT(0x141FC3180, 0x142B65680, 0x1445C3410)

namespace fb
{
    class ServerGameContext
    {
    public:
        char pad_0000[176];
        ServerPlayerManager* m_serverPlayerManager;
        ServerPeer* m_serverPeer;

        static ServerGameContext* GetInstance(void)
        {
            return *(ServerGameContext**)OFFSET_SERVERGAMECONTEXT;
        }
    };
}
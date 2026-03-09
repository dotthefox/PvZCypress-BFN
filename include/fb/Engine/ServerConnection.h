#pragma once
#include <EASTL/new_string.h>
#include <fb/SecureReason.h>

#define OFFSET_SERVERCONNECTION__ONCREATEPLAYERMESSAGE CYPRESS_GW_SELECT(0, 0x14064FA40, 0x140F4BBE0)

namespace fb
{
    class ServerConnection
    {
    public:
        char pad_0A10[0xA10];
        eastl::new_string m_machineId;
        char pad_0A3D[0x15];
        bool m_shouldDisconnect;
        SecureReason m_disconnectReason;
        eastl::new_string m_reasonText;

        void disconnect(SecureReason reason, const char* reasonStr)
        {
            m_reasonText = reasonStr;
            m_disconnectReason = reason;

            m_shouldDisconnect = true;
        }
    };
}
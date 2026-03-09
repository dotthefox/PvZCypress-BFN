#include "pch.h"
#include <Core/Program.h>
#include <Core/Logging.h>
#include <Core/Config.h>
#include <fb/Engine/Server.h>

void Cypress_LogToServer(const char* msg, const char* fileName, int lineNumber, LogLevel logLevel)
{
#if(HAS_DEDICATED_SERVER)
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);

    char timePrefix[64];
    strftime(timePrefix, sizeof(timePrefix), "[%d.%m.%Y %H:%M:%S]", tm_info);
#if _DEBUG

    const char* filePath = strrchr(fileName, '\\');
    if (filePath == nullptr)
        filePath = fileName;
    else
        ++filePath;

    std::string formattedLog = std::format("{} [{}] [{}:{}] {}", timePrefix, Cypress_LogLevelToStr(logLevel), filePath, lineNumber, msg);
#else
    std::string formattedLog = std::format("{} [{}] [Server] {}", timePrefix, Cypress_LogLevelToStr(logLevel), msg);
#endif

    Cypress::Server* pServer = g_program->GetServer();
    HWND listBox = pServer->GetListBox();

    if (listBox == NULL)
        std::cout << "\x1B[36m[SrvLog]" << formattedLog.c_str() << "\x1B[0m";
    else 
    {
        if (pServer->GetServerLogEnabled())
        {
            int pos = (int)SendMessageA(listBox, LB_ADDSTRING, 0, (LPARAM)formattedLog.c_str());
            if (pos >= 1000)
            {
                SendMessage(listBox, LB_DELETESTRING, 0, 0);
                pos--;
            }

            SendMessage(listBox, LB_SETCURSEL, pos, 1);
        }
    }
#endif //_HAS_DEDICATED_SERVER
}

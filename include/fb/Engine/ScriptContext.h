#pragma once
#include <string>

#define OFFSET_FB_SCRIPTCONTEXT_EXECUTESTRING 0x140446DC0

namespace fb
{
    class ScriptContext {
    public:
        static ScriptContext* context() {
            using tContext = ScriptContext * (*)();
            auto _context = reinterpret_cast<tContext>(0x140446190);

            return _context();
        }
        void executeString(const char* str) {
            using tScExecuteStr = void(*)(ScriptContext*, const char*, int, void*);
            auto scExecuteString = reinterpret_cast<tScExecuteStr>(OFFSET_FB_SCRIPTCONTEXT_EXECUTESTRING);

            scExecuteString(this, str, static_cast<int>(strlen(str)), nullptr);
        }
    };
}
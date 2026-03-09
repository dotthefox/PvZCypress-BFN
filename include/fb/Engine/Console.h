#pragma once

#define OFFSET_FB_CONSOLE_ENQUEUECOMMAND 0x14045A6C0

namespace fb
{
	class Console {
	public:

		static void enqueueCommand(const char* cmd)
		{
			void* callback[2]{ nullptr, nullptr }; //fb::FastDelegate

			using tEnqueueCommand = void(*)(const char*, void**);
			auto _enqueueCommand = reinterpret_cast<tEnqueueCommand>(OFFSET_FB_CONSOLE_ENQUEUECOMMAND);

			_enqueueCommand(cmd, callback);
		}
	};
}
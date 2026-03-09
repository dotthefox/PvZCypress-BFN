#pragma once
#include <IGameModule.h>

#ifdef CYPRESS_BFN
namespace Cypress
{
	class BFNModule : public IGameModule {
	public:
		void InitGameHooks() override;
		void InitMemPatches() override;
		void InitDedicatedServerPatches(class Cypress::Server* pServer) override;
		void RegisterCommands() override;
	};
}
#endif
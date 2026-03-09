#include "pch.h"
#include "fbClientHooks.h"
#include <fb/Engine/MessageListener.h>
#include <fb/Engine/ExecutionContext.h>
#include <Core/Program.h>

DEFINE_HOOK(
	fb_Client_enterState,
	__fastcall,
	void,

	void* thisPtr,
	fb::ClientState state,
	fb::ClientState prevState
)
{
	Cypress::Client* client = g_program->GetClient();
	client->SetFbClientInstance(thisPtr);
	client->SetClientState(state);

	if (state == fb::ClientState_ConnectToServer)
	{
		client->SetJoiningServer(true);
	}
	if (prevState == fb::ClientState_ConnectToServer)
	{
		client->SetJoiningServer(false);
	}

	if (state == fb::ClientState_Ingame && !client->AddedPrimaryUser()) //To be sure RimeUISystem has been initialized
	{
		client->AddPrimaryUser();
	}

	Orig_fb_Client_enterState(thisPtr, state, prevState);
}

DEFINE_HOOK(
	fb_OnlineManager_connectToAddress,
	__fastcall,
	void,

	void* thisPtr,
	const char* ipAddr,
	const char* serverPassword
)
{
	const char* passwordArg;
	if ((passwordArg = fb::ExecutionContext::getOptionValue("password")))
	{
		serverPassword = passwordArg;
	}
	Orig_fb_OnlineManager_connectToAddress(thisPtr, ipAddr, serverPassword);
}

DEFINE_HOOK(
	fb_OnlineManager_onGotDisconnected,
	__fastcall,
	void,

	void* thisPtr,
	fb::SecureReason reason,
	eastl::new_string* reasonText
)
{
	const char* reasonStr = "No reason Provided";

	if (!reasonText->empty())
		reasonStr = reasonText->c_str();

	std::string bodyMsg = std::format("Disconnect: {} ({})", reasonStr, fb::SecureReason_ToString(reason));

	if (reason == fb::SecureReason_TimedOut || reason == fb::SecureReason_NoReply || reason == fb::SecureReason_ConnectFailed)
	{
		if (MessageBoxA(GetActiveWindow(), bodyMsg.c_str(), "Disconnected", MB_ICONINFORMATION | MB_RETRYCANCEL) == IDRETRY)
			return Orig_fb_OnlineManager_onGotDisconnected(thisPtr, reason, reasonText);
	}
	else
	{
		MessageBoxA(GetActiveWindow(), bodyMsg.c_str(), "Disconnected", MB_ICONINFORMATION | MB_OK);
	}

	exit(0xCC1);
}

DEFINE_HOOK(
	fb_EAUser_ctor,
	__fastcall,
	void*,

	void* thisPtr,
	int localPlayerId,
	void* a3,
	void* controller
)
{
	void* pUser = Orig_fb_EAUser_ctor(thisPtr, localPlayerId, a3, controller);

	if (localPlayerId == 0)
		g_program->GetClient()->SetPrimaryUser(pUser);

	return pUser;
}
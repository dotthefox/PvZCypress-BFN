#pragma once
#include <fb/Engine/Client.h>
#include <fb/Engine/MessageListener.h>
#include <Kyber/SocketManager.h>

namespace Cypress
{
	class Client : public fb::MessageListener
	{
	public:
		Client();
		~Client();

		virtual void onMessage(fb::Message& inMessage) override;

		void* GetFbClientInstance() { return m_fbClientInstance; }
		void SetFbClientInstance(void* client) { m_fbClientInstance = client; }

		fb::ClientState GetClientState() { return m_clientState; }
		void SetClientState(fb::ClientState newState) { m_clientState = newState; }

		bool GetJoiningServer() { return m_joiningServer; }
		void SetJoiningServer(bool value) { m_joiningServer = value; }

		const char* GetPlayerName() { return m_playerName; }
		bool AddedPrimaryUser() { return m_addedPrimaryUser; }
		//BFN doesn´t register users by default, so we need to execute this to fix Profile Options and gamepad assignment
		void AddPrimaryUser();
		void SetPrimaryUser(void* user) { m_primaryUser = user; }

		Kyber::SocketManager* GetSocketManager() { return m_socketManager; }

	private:
		Kyber::SocketManager* m_socketManager;
		const char* m_playerName;
		void* m_primaryUser;
		void* m_fbClientInstance;
		fb::ClientState m_clientState;
		bool m_joiningServer;
		bool m_addedPrimaryUser;

		friend class Program;
	};
}
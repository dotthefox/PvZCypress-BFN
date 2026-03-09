#include "pch.h"
#include "Client.h"
#include <fb/Engine/Message.h>
#include <fb/Engine/TypeInfo.h>

#define OFFSET_FB_CLIENTPLAYERSELECTENTITY_ADDPERMANENTUSER 0x1417A3910

namespace Cypress
{
	Client::Client()
		: m_socketManager(new Kyber::SocketManager(Kyber::ProtocolDirection::Serverbound, Kyber::SocketSpawnInfo(false, "", "")))
		, m_playerName(nullptr)
		, m_primaryUser(nullptr)
		, m_fbClientInstance(nullptr)
		, m_clientState(fb::ClientState::ClientState_None)
		, m_joiningServer(false)
		, m_addedPrimaryUser(false)
	{

	}

	Client::~Client()
	{
	}

	void Client::onMessage(fb::Message& inMessage)
	{

	}

	void Client::AddPrimaryUser()
	{
		if (m_primaryUser == nullptr) return;

		using tAddPrimaryUser = void(*)(void*, void**, unsigned int);
		auto addPrimaryUser = reinterpret_cast<tAddPrimaryUser>(OFFSET_FB_CLIENTPLAYERSELECTENTITY_ADDPERMANENTUSER);

		addPrimaryUser(nullptr, &m_primaryUser, 0);

		m_addedPrimaryUser = true;
	}
}

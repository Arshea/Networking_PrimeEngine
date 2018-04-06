#include "ServerGameObjectManagerAddon.h"

#include "PrimeEngine/Lua/Server/ServerLuaEnvironment.h"
#include "PrimeEngine/Networking/Server/ServerNetworkManager.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"

#include "Characters/SoldierNPC.h"
#include "WayPoint.h"
#include "Tank/ClientTank.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;
using namespace CharacterControl::Components;

namespace CharacterControl{
namespace Components
{
PE_IMPLEMENT_CLASS1(ServerGameObjectManagerAddon, GameObjectManagerAddon); // creates a static handle and GteInstance*() methods. still need to create construct

void ServerGameObjectManagerAddon::addDefaultComponents()
{
	GameObjectManagerAddon::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Event_MoveTank_C_to_S, ServerGameObjectManagerAddon::do_MoveTank);
	PE_REGISTER_EVENT_HANDLER(ServerTankAnimSM_Event_WALK, ServerGameObjectManagerAddon::do_WalkAnimation);
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_CLIENT_SERVER_UDP_ACK, ServerGameObjectManagerAddon::do_CLIENT_SERVER_UDP_ACK);
	PE_REGISTER_EVENT_HANDLER(Event_Server_Win, ServerGameObjectManagerAddon::do_Win);
}

void ServerGameObjectManagerAddon::do_MoveTank(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_MoveTank_C_to_S>());

	Event_MoveTank_C_to_S *pTrueEvent = (Event_MoveTank_C_to_S*)(pEvt);

	// need to send this event to all clients except the client it came from

	Event_MoveTank_S_to_C fwdEvent(*m_pContext);
	fwdEvent.m_transform = pTrueEvent->m_transform;
	fwdEvent.m_clientTankId = pTrueEvent->m_networkClientId; // need to tell cleints which tank to move

	ServerNetworkManager *pNM = (ServerNetworkManager *)(m_pContext->getNetworkManager());
	pNM->scheduleEventToAllExcept(&fwdEvent, m_pContext->getGameObjectManager(), pTrueEvent->m_networkClientId);

	if (SERVER_MOVEMENT) {
		Event_Tank_Throttle_From_Server moveThisTank(*m_pContext);
		moveThisTank.m_transform = pTrueEvent->m_transform;
		moveThisTank.m_relativeMove = pTrueEvent->m_relativeMove;
		moveThisTank.m_frameTime = pTrueEvent->m_frameTime;

		pNM->scheduleEventTo(&moveThisTank, m_pContext->getGameObjectManager(), pTrueEvent->m_networkClientId);

	}


}
void ServerGameObjectManagerAddon::do_WalkAnimation(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<ServerTankAnimSM_Event_WALK>());

	ServerTankAnimSM_Event_WALK *pTrueEvent = (ServerTankAnimSM_Event_WALK*)(pEvt);

	// need to send this event to all clients except the client it came from

	Event_Client_Walk fwdEvent(*m_pContext);
	//fwdEvent.m_transform = pTrueEvent->m_transform;
	fwdEvent.m_clientTankId = pTrueEvent->m_networkClientId; // need to tell cleints which tank to move

	ServerNetworkManager *pNM = (ServerNetworkManager *)(m_pContext->getNetworkManager());
	pNM->scheduleEventToAllExcept(&fwdEvent, m_pContext->getGameObjectManager(), pTrueEvent->m_networkClientId);

}
 
void ServerGameObjectManagerAddon::do_Win(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_Server_Win>());

	Event_Server_Win *pTrueEvent = (Event_Server_Win*)(pEvt);

	// need to send this event to all clients except the client it came from

	Event_Client_Lose loseEvent(*m_pContext);
	//fwdEvent.m_transform = pTrueEvent->m_transform;
	loseEvent.m_clientTankId = pTrueEvent->m_networkClientId; // need to tell cleints which player won

	Event_Client_Win winEvent(*m_pContext);
	ServerNetworkManager *pNM = (ServerNetworkManager *)(m_pContext->getNetworkManager());
	//Tell all other clients they lost
	pNM->scheduleEventToAllExcept(&loseEvent, m_pContext->getGameObjectManager(), pTrueEvent->m_networkClientId);
	//Tell winner they won
	pNM->scheduleEventTo(&winEvent, m_pContext->getGameObjectManager(), pTrueEvent->m_networkClientId);
	

}

void ServerGameObjectManagerAddon::do_CLIENT_SERVER_UDP_ACK(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CLIENT_SERVER_UDP_ACK>());

	Event_CLIENT_SERVER_UDP_ACK *pTrueEvent = (Event_CLIENT_SERVER_UDP_ACK*)(pEvt);
}

}
}

#include "ClientGameObjectManagerAddon.h"

#include "PrimeEngine/PrimeEngineIncludes.h"

#include "Characters/SoldierNPC.h"
#include "WayPoint.h"
#include "Tank/ClientTank.h"
#include "CharacterControl/Client/ClientSpaceShip.h"
#include "Tank/ClientTankAnimationSM.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;
using namespace CharacterControl::Components;

namespace CharacterControl{
namespace Components
{
PE_IMPLEMENT_CLASS1(ClientGameObjectManagerAddon, Component); // creates a static handle and GteInstance*() methods. still need to create construct

void ClientGameObjectManagerAddon::addDefaultComponents()
{
	GameObjectManagerAddon::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Event_CreateSoldierNPC, ClientGameObjectManagerAddon::do_CreateSoldierNPC);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_WAYPOINT, ClientGameObjectManagerAddon::do_CREATE_WAYPOINT);

	// note this component (game obj addon) is added to game object manager after network manager, so network manager will process this event first
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_SERVER_CLIENT_CONNECTION_ACK, ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK);

	PE_REGISTER_EVENT_HANDLER(Event_MoveTank_S_to_C, ClientGameObjectManagerAddon::do_MoveTank);
	PE_REGISTER_EVENT_HANDLER(Event_Client_Lose, ClientGameObjectManagerAddon::do_Lose);
	PE_REGISTER_EVENT_HANDLER(Event_Client_Win, ClientGameObjectManagerAddon::do_Win);
	PE_REGISTER_EVENT_HANDLER(Event_Client_Walk, ClientGameObjectManagerAddon::do_WalkAnimation);
}

void ClientGameObjectManagerAddon::do_CreateSoldierNPC(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateSoldierNPC>());

	Event_CreateSoldierNPC *pTrueEvent = (Event_CreateSoldierNPC*)(pEvt);

	createSoldierNPC(pTrueEvent);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Vector3 pos, int &threadOwnershipMask)
{
	Event_CreateSoldierNPC evt(threadOwnershipMask);
	evt.m_pos = pos;
	evt.m_u = Vector3(1.0f, 0, 0);
	evt.m_v = Vector3(0, 1.0f, 0);
	evt.m_n = Vector3(0, 0, 1.0f);
	
	StringOps::writeToString( "SoldierTransform.mesha", evt.m_meshFilename, 255);
	StringOps::writeToString( "Soldier", evt.m_package, 255);
	StringOps::writeToString( "mg34.x_mg34main_mesh.mesha", evt.m_gunMeshName, 64);
	StringOps::writeToString( "CharacterControl", evt.m_gunMeshPackage, 64);
	StringOps::writeToString( "", evt.m_patrolWayPoint, 32);
	createSoldierNPC(&evt);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Event_CreateSoldierNPC *pTrueEvent)
{
	PEINFO("CharacterControl: GameObjectManagerAddon: Creating CreateSoldierNPC\n");

	PE::Handle hSoldierNPC("SoldierNPC", sizeof(SoldierNPC));
	SoldierNPC *pSoldierNPC = new(hSoldierNPC) SoldierNPC(*m_pContext, m_arena, hSoldierNPC, pTrueEvent);
	pSoldierNPC->addDefaultComponents();

	// add the soldier as component to the ObjecManagerComponentAddon
	// all objects of this demo live in the ObjecManagerComponentAddon
	addComponent(hSoldierNPC);
}

void ClientGameObjectManagerAddon::do_CREATE_WAYPOINT(PE::Events::Event *pEvt)
{
	PEINFO("GameObjectManagerAddon::do_CREATE_WAYPOINT()\n");

	assert(pEvt->isInstanceOf<Event_CREATE_WAYPOINT>());

	Event_CREATE_WAYPOINT *pTrueEvent = (Event_CREATE_WAYPOINT*)(pEvt);

	PE::Handle hWayPoint("WayPoint", sizeof(WayPoint));
	WayPoint *pWayPoint = new(hWayPoint) WayPoint(*m_pContext, m_arena, hWayPoint, pTrueEvent);
	pWayPoint->addDefaultComponents();

	addComponent(hWayPoint);
}

WayPoint *ClientGameObjectManagerAddon::getWayPoint(const char *name)
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<WayPoint>())
		{
			WayPoint *pWP = (WayPoint *)(pC);
			if (StringOps::strcmp(pWP->m_name, name) == 0)
			{
				// equal strings, found our waypoint
				return pWP;
			}
		}
	}
	return NULL;
}

SoldierNPC *ClientGameObjectManagerAddon::getTarget(const char *name)
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<SoldierNPC>())
		{
			SoldierNPC *pSol = (SoldierNPC *)(pC);
			OutputDebugStringA("Now comparing target name ");
			OutputDebugStringA(name);
			OutputDebugStringA(" with  ");
			OutputDebugStringA(pSol->m_isTargetName);
			OutputDebugStringA("\n");
			if (StringOps::strcmp(pSol->m_isTargetName, name) == 0)
			{
				// equal strings, found our target
				return pSol;
			}
		}
	}
	return NULL;
}

void ClientGameObjectManagerAddon::createTank(int index, int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    Tank

	//game object manager
	//  TankController
	//    scene node
	//m_pContext->getGPUScreen()->AcquireRenderContextOwnership(threadOwnershipMask);
	////////////////Added for debug renderer in win event////////////
	m_threadOwnershipMask = threadOwnershipMask;
	////////////////////////////////////////////////////////////////
	
	//mesh instance
	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
	pMeshInstance->addDefaultComponents();
	//pMeshInstance->initFromFile("jack.mesha", "Ninja", threadOwnershipMask);
	pMeshInstance->initFromFile("Vampire.mesha","Vampire2",threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	//animation SM
	PE::Handle hSoldierAnimSM("ClientTankAnimationSM", sizeof(ClientTankAnimationSM));
	ClientTankAnimationSM *pSoldierAnimSM = new(hSoldierAnimSM) ClientTankAnimationSM(*m_pContext, m_arena, hSoldierAnimSM);
	pSoldierAnimSM->addDefaultComponents();
	pSoldierAnimSM->m_debugAnimIdOffset = 0;

	addComponent(hSoldierAnimSM);

	//Skeleton Instance
	PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
	SkeletonInstance *pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance,
		hSoldierAnimSM);
	pSkelInst->addDefaultComponents();
	
	pSkelInst->initFromFiles("vampire-t-pose_Hips.skela", "Vampire2", threadOwnershipMask);

	pSkelInst->setAnimSet("newAnimSet_Hips.animseta", "Vampire2");
	//pSkelInst->initFromFiles("NinjaSkeleton_mixamorig_Hips.skela", "Ninja", threadOwnershipMask);

	//pSkelInst->setAnimSet("Run.animseta", "Ninja");
	
	////////////////////////////////////////////////////////////////////////////////
	// DOING A THING (Physics Testing)
	////////////////////////////////////////////////////////////////////////////////
	PositionBufferCPU *pbuf = pMeshInstance->m_hAsset.getObject<Mesh>()->m_hPositionBufferCPU.getObject<PositionBufferCPU>();

	PrimitiveTypes::Float32 min[3];
	PrimitiveTypes::Float32 max[3];

	// Initialise
	for (int i = 0; i < 3; i++) {
		min[i] = max[i] = pbuf->m_values[i];
	}

	// Get mins and maxes
	for (int i = 3; i < pbuf->m_values.m_size; i += 3) {
		for (int j = 0; j < 3; j++) {
			min[j] = min[j] < pbuf->m_values[i + j] ? min[j] : pbuf->m_values[i + j];
			max[j] = max[j] > pbuf->m_values[i + j] ? max[j] : pbuf->m_values[i + j];
		}
	}

	Matrix4x4 base = pSN->m_base;

	// Add base to min and max points; store in imrod mesh
	pMeshInstance->min[0] = min[0] + base.getPos().getX();
	pMeshInstance->min[1] = min[1] + base.getPos().getY();
	pMeshInstance->min[2] = min[2] + base.getPos().getZ();
	pMeshInstance->max[0] = max[0] + base.getPos().getX();
	pMeshInstance->max[1] = max[1] + base.getPos().getY();
	pMeshInstance->max[2] = max[2] + base.getPos().getZ();

	bool isDynamic = true;
	PhysicsComponent *newPhys = new PhysicsComponent(pMeshInstance->min, pMeshInstance->max, base, isDynamic, none);
	//pSN->my_phys = *newPhys;
	//////////////////////////////////////////////////////////////////////////
	//								DID A THING								//
	//////////////////////////////////////////////////////////////////////////

	Vector3 spawnPos(0.0f + 6.0f * index, 0 , 0.0f);
	pSN->m_base.setPos(spawnPos);
	pSkelInst->addComponent(hMeshInstance);
	//pSN->addComponent(hMeshInstance);
	pSN->addComponent(hSkeletonInstance);
	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hTankController("TankController", sizeof(TankController));
	TankController *pTankController = new(hTankController) TankController(*m_pContext, m_arena, hTankController, 0.05f, spawnPos,  0.05f);
	pTankController->addDefaultComponents();

	addComponent(hTankController);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
	pTankController->addComponent(hSN, &alllowedEventsToPropagate[0]);

	pTankController->my_phys = *newPhys;
	//m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_threadOwnershipMask);
}

void ClientGameObjectManagerAddon::createSpaceShip(int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    SpaceShip

	//game object manager
	//  SpaceShipController
	//    scene node

	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	pMeshInstance->initFromFile("space_frigate_6.mesha", "FregateTest", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(0, 0, 0.0f);
	pSN->m_base.setPos(spawnPos);

	pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hSpaceShip("ClientSpaceShip", sizeof(ClientSpaceShip));
	ClientSpaceShip *pSpaceShip = new(hSpaceShip) ClientSpaceShip(*m_pContext, m_arena, hSpaceShip, 0.05f, spawnPos,  0.05f);
	pSpaceShip->addDefaultComponents();

	addComponent(hSpaceShip);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of space ship just for referencing purposes
	pSpaceShip->addComponent(hSN, &alllowedEventsToPropagate[0]);

	pSpaceShip->activate();
}


void ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK(PE::Events::Event *pEvt)
{
	Event_SERVER_CLIENT_CONNECTION_ACK *pRealEvt = (Event_SERVER_CLIENT_CONNECTION_ACK *)(pEvt);
	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pRealEvt->m_clientId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->activate();
				break;
			}
			++itc;
		}
	}
}

void ClientGameObjectManagerAddon::do_MoveTank(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_MoveTank_S_to_C>());

	Event_MoveTank_S_to_C *pTrueEvent = (Event_MoveTank_S_to_C*)(pEvt);

	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pTrueEvent->m_clientTankId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->overrideTransform(pTrueEvent->m_transform);
				PE::Handle r("WALKEVENT", sizeof(Events::ClientTankAnimSM_Event_WALK));
				Events::ClientTankAnimSM_Event_WALK *walkEvt = new(r) Events::ClientTankAnimSM_Event_WALK;
				PE::Handle hFisrtSN = pTK->getFirstComponentHandle<SceneNode>();
				if (!hFisrtSN.isValid())
				{
					assert(!"wrong setup. must have scene node referenced");
					return;
				}
				SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
				//animation thing
				pFirstSN->handleEvent(walkEvt);
			//	PE::Events::EventQueueManager::Instance()->add(r, QT_GENERAL);
				//hFisrtSN.release();
				r.release();
				break;
			}
			++itc;
		}
	}
}


void ClientGameObjectManagerAddon::do_Lose(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_Client_Lose>());

	Event_Client_Lose *pTrueEvent = (Event_Client_Lose*)(pEvt);

	
	char *buf = "You lose!";
	DebugRenderer::Instance()->createTextMesh(buf, true, false, false, false, 1000000, Vector3(.40f, .45f, 0), 3.0f, m_threadOwnershipMask);
	
}

void ClientGameObjectManagerAddon::do_Win(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_Client_Win>());

	Event_Client_Win *pTrueEvent = (Event_Client_Win*)(pEvt);


	char *buf = "You win!";
	DebugRenderer::Instance()->createTextMesh(buf, true, false, false, false, 1000000, Vector3(.40f, .45f, 0), 3.0f, m_threadOwnershipMask);

}

void ClientGameObjectManagerAddon::do_WalkAnimation(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf < Event_Client_Walk > ());

	Event_Client_Walk *pTrueEvent = (Event_Client_Walk*)(pEvt);
	char *buf = "Walking";
	DebugRenderer::Instance()->createTextMesh(buf, true, false, false, false, 1000000, Vector3(.40f, .45f, 0), 3.0f, m_threadOwnershipMask);

}


}
}

#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/RootSceneNode.h"

#include "SoldierNPC.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPCMovementSM.h"
#include "SoldierNPCBehaviorSM.h"

// Assignment 4 Debug renderer
#include "PrimeEngine/Scene/DebugRenderer.h"

using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl{
namespace Components {

PE_IMPLEMENT_CLASS1(SoldierNPC, Component);

SoldierNPC::SoldierNPC(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Event_CreateSoldierNPC *pEvt) : Component(context, arena, hMyself)
{

	// hierarchy of soldier and replated components and variables (note variables are just variables, they are not passed events to)
	// scene
	// +-components
	//   +-soldier scene node
	//   | +-components
	//   |   +-soldier skin
	//   |     +-components
	//   |       +-soldier animation state machine
	//   |       +-soldier weapon skin scene node
	//   |         +-components
	//   |           +-weapon mesh

	// game objects
	// +-components
	//   +-soldier npc
	//     +-variables
	//     | +-m_hMySN = soldier scene node
	//     | +-m_hMySkin = skin
	//     | +-m_hMyGunSN = soldier weapon skin scene node
	//     | +-m_hMyGunMesh = weapon mesh
	//     +-components
	//       +-soldier scene node (restricted to no events. this is for state machines to be able to locate the scene node)
	//       +-movement state machine
	//       +-behavior state machine

    
	// need to acquire redner context for this code to execute thread-safe
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pEvt->m_threadOwnershipMask);
	
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pMainSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pMainSN->addDefaultComponents();

	pMainSN->m_base.setPos(pEvt->m_pos);
	pMainSN->m_base.setU(pEvt->m_u);
	pMainSN->m_base.setV(pEvt->m_v);
	pMainSN->m_base.setN(pEvt->m_n);


	RootSceneNode::Instance()->addComponent(hSN);

	// add the scene node as component of soldier without any handlers. this is just data driven way to locate scnenode for soldier's components
	{
		static int allowedEvts[] = {0};
		addComponent(hSN, &allowedEvts[0]);
	}

	int numskins = 1; // 8
	for (int iSkin = 0; iSkin < numskins; ++iSkin)
	{
		float z = (iSkin / 4) * 1.5f;
		float x = (iSkin % 4) * 1.5f;
		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		pSN->m_base.setPos(Vector3(x, 0, z));
		
		// rotation scene node to rotate soldier properly, since soldier from Maya is facing wrong direction
		PE::Handle hRotateSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pRotateSN = new(hRotateSN) SceneNode(*m_pContext, m_arena, hRotateSN);
		pRotateSN->addDefaultComponents();

		pSN->addComponent(hRotateSN);

		pRotateSN->m_base.turnLeft(3.1415);

		PE::Handle hSoldierAnimSM("SoldierNPCAnimationSM", sizeof(SoldierNPCAnimationSM));
		SoldierNPCAnimationSM *pSoldierAnimSM = new(hSoldierAnimSM) SoldierNPCAnimationSM(*m_pContext, m_arena, hSoldierAnimSM);
		pSoldierAnimSM->addDefaultComponents();

		pSoldierAnimSM->m_debugAnimIdOffset = 0;// rand() % 3;

		PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
		SkeletonInstance *pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance, 
			hSoldierAnimSM);
		pSkelInst->addDefaultComponents();

		pSkelInst->initFromFiles("soldier_Soldier_Skeleton.skela", "Soldier", pEvt->m_threadOwnershipMask);

		pSkelInst->setAnimSet("soldier_Soldier_Skeleton.animseta", "Soldier");

		PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
		MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();
		
		pMeshInstance->initFromFile(pEvt->m_meshFilename, pEvt->m_package, pEvt->m_threadOwnershipMask);
		
		////////////////////////////////////////////////////////////////////////////////
		// DOING A THING (Assignment 4)
		////////////////////////////////////////////////////////////////////////////////
		PositionBufferCPU *pbuf = pMeshInstance->m_hAsset.getObject<Mesh>()->m_hPositionBufferCPU.getObject<PositionBufferCPU>();
		// Min and Max coordinates for AABB
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

		Matrix4x4 base = pMainSN->m_base;

		//// Add base to min and max points; store in imrod mesh
		//pMeshInstance->min[0] = min[0] + base.getPos().getX();
		//pMeshInstance->min[1] = min[1] + base.getPos().getY();
		//pMeshInstance->min[2] = min[2] + base.getPos().getZ();
		//pMeshInstance->max[0] = max[0] + base.getPos().getX();
		//pMeshInstance->max[1] = max[1] + base.getPos().getY();
		//pMeshInstance->max[2] = max[2] + base.getPos().getZ();

		//bool isDynamic = true;
		//PhysicsComponent *newPhys = new PhysicsComponent(pMeshInstance->min, pMeshInstance->max, base, isDynamic);
		//my_physics_component = *newPhys;


		//////////////////////////////////////////////////////////////////////////
		//								DID A THING								//
		//////////////////////////////////////////////////////////////////////////

		pSkelInst->addComponent(hMeshInstance);

		// add skin to scene node
		pRotateSN->addComponent(hSkeletonInstance);

		#if !APIABSTRACTION_D3D11
		{
			PE::Handle hMyGunMesh = PE::Handle("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pGunMeshInstance = new(hMyGunMesh) MeshInstance(*m_pContext, m_arena, hMyGunMesh);

			pGunMeshInstance->addDefaultComponents();
			pGunMeshInstance->initFromFile(pEvt->m_gunMeshName, pEvt->m_gunMeshPackage, pEvt->m_threadOwnershipMask);

			// create a scene node for gun attached to a joint

			PE::Handle hMyGunSN = PE::Handle("SCENE_NODE", sizeof(JointSceneNode));
			JointSceneNode *pGunSN = new(hMyGunSN) JointSceneNode(*m_pContext, m_arena, hMyGunSN, 38);
			pGunSN->addDefaultComponents();

			// add gun to joint
			pGunSN->addComponent(hMyGunMesh);

			// add gun scene node to the skin
			pSkelInst->addComponent(hMyGunSN);
		}
		#endif
				
		pMainSN->addComponent(hSN);
	}

	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pEvt->m_threadOwnershipMask);
	
#if 1
	// add movement state machine to soldier npc
    PE::Handle hSoldierMovementSM("SoldierNPCMovementSM", sizeof(SoldierNPCMovementSM));
	SoldierNPCMovementSM *pSoldierMovementSM = new(hSoldierMovementSM) SoldierNPCMovementSM(*m_pContext, m_arena, hSoldierMovementSM);
	pSoldierMovementSM->addDefaultComponents();
	pSoldierMovementSM->myPhysics = my_physics_component;

	// add it to soldier NPC
	addComponent(hSoldierMovementSM);

	// add behavior state machine ot soldier npc
    PE::Handle hSoldierBehaviorSM("SoldierNPCBehaviorSM", sizeof(SoldierNPCBehaviorSM));
	SoldierNPCBehaviorSM *pSoldierBehaviorSM = new(hSoldierBehaviorSM) SoldierNPCBehaviorSM(*m_pContext, m_arena, hSoldierBehaviorSM, hSoldierMovementSM);
	pSoldierBehaviorSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hSoldierBehaviorSM);

	StringOps::writeToString(pEvt->m_patrolWayPoint, pSoldierBehaviorSM->m_curPatrolWayPoint, 32);
	pSoldierBehaviorSM->m_havePatrolWayPoint = StringOps::length(pSoldierBehaviorSM->m_curPatrolWayPoint) > 0;

	// add target if exists
	StringOps::writeToString(pEvt->m_getsTargetName, pSoldierBehaviorSM->m_getsTargetName, 32);
	pSoldierBehaviorSM->m_hasTarget = StringOps::length(pSoldierBehaviorSM->m_getsTargetName) > 0;

	// add isTarget name if exists
	StringOps::writeToString(pEvt->m_isTargetName, pSoldierBehaviorSM->m_isTargetName, 32);

	// start the soldier
	pSoldierBehaviorSM->start();
#endif
}

void SoldierNPC::addDefaultComponents()
{
	Component::addDefaultComponents();

	// custom methods of this component
}

}; // namespace Components
}; // namespace CharacterControl

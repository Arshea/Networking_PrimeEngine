// APIAbstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "../Lua/LuaEnvironment.h"

// Sibling/Children includes
#include "GameObjectManager.h"
#include "../Sound/SoundManager.h"

#include "PrimeEngine/Scene/Skeleton.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/DebugRenderer.h"

// Assignment 4
#include "PrimeEngine/Physics/PhysicsComponent.h"
#include "PrimeEngine/Physics/Shape.h"

namespace PE {
namespace Components {

using namespace PE::Events;

PE_IMPLEMENT_CLASS1(GameObjectManager, Component);

// Singleton ------------------------------------------------------------------

// Constructor -------------------------------------------------------------
GameObjectManager::GameObjectManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
: Component(context, arena, hMyself), m_luaGameObjectTableRef(LUA_NOREF)
, Networkable(context, this, Networkable::s_NetworkId_GameObjectManager) // pre-assigned network id
{
}

	// Methods      ------------------------------------------------------------
void GameObjectManager::addDefaultComponents()
{
	Component::addDefaultComponents();

	addComponent(m_pContext->getLuaEnvironment()->getHandle());

	PE_REGISTER_EVENT_HANDLER(Event_SET_DEBUG_TARGET_HANDLE, GameObjectManager::do_SET_DEBUG_TARGET_HANDLE);
	PE_REGISTER_EVENT_HANDLER(Event_CONSTRUCT_SOUND, GameObjectManager::do_CONSTRUCT_SOUND);

	PE_REGISTER_EVENT_HANDLER(Event_CREATE_LIGHT, GameObjectManager::do_CREATE_LIGHT);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_MESH, GameObjectManager::do_CREATE_MESH);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_SKELETON, GameObjectManager::do_CREATE_SKELETON);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_ANIM_SET, GameObjectManager::do_CREATE_ANIM_SET);

	createGameObjectTableIfDoesntExist();
}

// Individual events -------------------------------------------------------
void GameObjectManager::do_SET_DEBUG_TARGET_HANDLE(Events::Event *pEvt)
{
	Event_SET_DEBUG_TARGET_HANDLE *pRealEvt = (Event_SET_DEBUG_TARGET_HANDLE *)(pEvt);

	Component::s_debuggedComponent = pRealEvt->m_hDebugTarget;
	Component::s_debuggedEvent = pRealEvt->m_debugEvent;
}

void GameObjectManager::do_CREATE_LIGHT(Events::Event *pEvt)
{
	Event_CREATE_LIGHT *pRealEvt = (Event_CREATE_LIGHT *)(pEvt);

	bool haveObject = false;
	Handle exisitngObject;

	putGameObjectTableIOnStack();

	if (!pRealEvt->m_peuuid.isZero())
	{
		// have a valid peeuid for the object. need to check if have one already

		haveObject = m_pContext->getLuaEnvironment()->checkTableValueByPEUUIDFieldExists(pRealEvt->m_peuuid);
		if (haveObject)
		{
			LuaEnvironment::popHandleFromTableOnStackAndPopTable(m_pContext->getLuaEnvironment()->L, exisitngObject);
			m_lastAddedObjHandle = exisitngObject;
		}
		else
		{
			// pop nil
			m_pContext->getLuaEnvironment()->pop();
		}
	}

	if (!haveObject)
	{
		Handle hLight("LIGHT", sizeof(Light));

		Light *pLight = new(hLight) Light(
			*m_pContext,
			m_arena,
			hLight,
			pRealEvt->m_pos, //Position
			pRealEvt->m_u, 
			pRealEvt->m_v, 
			pRealEvt->m_n, //Direction (z-axis)
			pRealEvt->m_ambient, //Ambient
			pRealEvt->m_diffuse, //Diffuse
			pRealEvt->m_spec, //Specular
			pRealEvt->m_att, //Attenuation (x, y, z)
			pRealEvt->m_spotPower, // Spot Power
			pRealEvt->m_range, //Range
			pRealEvt->m_isShadowCaster, //Whether or not it casts shadows
			(PrimitiveTypes::Int32)(pRealEvt->m_type) //0 = point, 1 = directional, 2 = spot
		);
		pLight->addDefaultComponents();

		RootSceneNode::Instance()->m_lights.add(hLight);
		RootSceneNode::Instance()->addComponent(hLight);

		m_pContext->getLuaEnvironment()->pushHandleAsFieldAndSet(pRealEvt->m_peuuid, hLight);
		m_lastAddedObjHandle = hLight;
	}
	else
	{
		// already have this object
		
		// need to reset the orientation
		// and light source settings
		Light *pLight = exisitngObject.getObject<Light>();
		
		pLight->m_base.setPos(pRealEvt->m_pos);
		pLight->m_base.setU(pRealEvt->m_u);
		pLight->m_base.setV(pRealEvt->m_v);
		pLight->m_base.setN(pRealEvt->m_n);
		

		pLight->m_cbuffer.pos = pLight->m_base.getPos();
		pLight->m_cbuffer.dir = pLight->m_base.getN();

		pLight->m_cbuffer.ambient = pRealEvt->m_ambient;
		pLight->m_cbuffer.diffuse = pRealEvt->m_diffuse;
		pLight->m_cbuffer.spec = pRealEvt->m_spec;
		pLight->m_cbuffer.att = pRealEvt->m_att;
		pLight->m_cbuffer.spotPower = pRealEvt->m_spotPower;
		pLight->m_cbuffer.range = pRealEvt->m_range;
		pLight->isTheShadowCaster = pRealEvt->m_isShadowCaster;
	}

	// pop the game object table
	m_pContext->getLuaEnvironment()->pop();
}

void GameObjectManager::do_CREATE_SKELETON(Events::Event *pEvt)
{
	Events::Event_CREATE_SKELETON *pRealEvent = (Events::Event_CREATE_SKELETON *)(pEvt);
	bool haveObject = false;
	Handle exisitngObject;

	putGameObjectTableIOnStack();

	if (!pRealEvent->m_peuuid.isZero())
	{
		// have a valid peeuid for the object. need to check if have one already

		haveObject = m_pContext->getLuaEnvironment()->checkTableValueByPEUUIDFieldExists(pRealEvent->m_peuuid);
		if (haveObject)
		{
			LuaEnvironment::popHandleFromTableOnStackAndPopTable(m_pContext->getLuaEnvironment()->L, exisitngObject);
			m_lastAddedObjHandle = exisitngObject;
		}
		else
		{
			// pop nil
			m_pContext->getLuaEnvironment()->pop();
		}

	}

	if (!haveObject)
	{
		// need to acquire redner context for this code to execute thread-safe
		m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pRealEvent->m_threadOwnershipMask);

		PE::Handle hSkelInstance("SkeletonInstance", sizeof(SkeletonInstance));
		SkeletonInstance *pSkelInstance = new(hSkelInstance) SkeletonInstance(*m_pContext, m_arena, hSkelInstance, Handle());
		pSkelInstance->addDefaultComponents();

		pSkelInstance->initFromFiles(pRealEvent->m_skelFilename, pRealEvent->m_package, pRealEvent->m_threadOwnershipMask);

		m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pRealEvent->m_threadOwnershipMask);
		
		if (pRealEvent->hasCustomOrientation)
		{
			// need to create a scene node for this mesh
			Handle hSN("SCENE_NODE", sizeof(SceneNode));
			SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
			pSN->addDefaultComponents();

			pSN->m_base.setPos(pRealEvent->m_pos);
			pSN->m_base.setU(pRealEvent->m_u);
			pSN->m_base.setV(pRealEvent->m_v);
			pSN->m_base.setN(pRealEvent->m_n);

			pSN->addComponent(hSkelInstance);

			RootSceneNode::Instance()->addComponent(hSN);
		}
		else
		{
			RootSceneNode::Instance()->addComponent(hSkelInstance);
		}
		m_pContext->getLuaEnvironment()->pushHandleAsFieldAndSet(pRealEvent->m_peuuid, hSkelInstance);
		m_lastAddedObjHandle = hSkelInstance;
		m_lastAddedSkelInstanceHandle = hSkelInstance;
	}
	else
	{
		// already have this object
		// only care about orientation
		if (pRealEvent->hasCustomOrientation)
		{
			// need to reset the orientation
			// try finding scene node
			SkeletonInstance *pSkelInstance = exisitngObject.getObject<SkeletonInstance>();
			Handle hSN = pSkelInstance->getFirstParentByType<SceneNode>();
			if (hSN.isValid())
			{
				SceneNode *pSN = hSN.getObject<SceneNode>();
				pSN->m_base.setPos(pRealEvent->m_pos);
				pSN->m_base.setU(pRealEvent->m_u);
				pSN->m_base.setV(pRealEvent->m_v);
				pSN->m_base.setN(pRealEvent->m_n);
			}
		}
	}
	
	// pop the game object table
	m_pContext->getLuaEnvironment()->pop();
}

void GameObjectManager::do_CREATE_ANIM_SET(Events::Event *pEvt)
{
	Events::Event_CREATE_ANIM_SET *pRealEvent = (Events::Event_CREATE_ANIM_SET *)(pEvt);
	bool haveObject = false;
	Handle exisitngObject;

	putGameObjectTableIOnStack();

	if (!haveObject)
	{
		// need to acquire redner context for this code to execute thread-safe
		m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pRealEvent->m_threadOwnershipMask);

		PEASSERT(m_lastAddedSkelInstanceHandle.isValid(), "Adding anim set, so we need a skeleton instance");
		m_lastAddedSkelInstanceHandle.getObject<SkeletonInstance>()->setAnimSet(pRealEvent->animSetFilename, pRealEvent->m_package);
		m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pRealEvent->m_threadOwnershipMask);
	}
	// pop the game object table
	m_pContext->getLuaEnvironment()->pop();
}

void GameObjectManager::do_CREATE_MESH(Events::Event *pEvt)
{
	OutputDebugStringA("Doing a thing in GameObjectManager !!!!!!!!!! \n");
	Events::Event_CREATE_MESH *pRealEvent = (Events::Event_CREATE_MESH *)(pEvt);
	
	bool haveObject = false;
	bool haveOtherObject = false;
	Handle exisitngObject;

	putGameObjectTableIOnStack();

	if (!pRealEvent->m_peuuid.isZero())
	{

		// have a valid peeuid for the object. need to check if have one already
		
		haveObject = m_pContext->getLuaEnvironment()->checkTableValueByPEUUIDFieldExists(pRealEvent->m_peuuid);
		if (haveObject)
		{
			LuaEnvironment::popHandleFromTableOnStackAndPopTable(m_pContext->getLuaEnvironment()->L, exisitngObject);
		}
		else
		{
			// pop nil
			m_pContext->getLuaEnvironment()->pop();
		}

		if (haveObject)
		{
			Component *pExisiting = exisitngObject.getObject<Component>();

			if (!pExisiting->isInstanceOf<MeshInstance>())
			{
				haveObject = false; // objects can have same id if they are different types, like skeleton + mesh
				haveOtherObject = true;
			}
		}
	}

	if (!haveObject)
	{

		// need to acquire redner context for this code to execute thread-safe
		m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pRealEvent->m_threadOwnershipMask);
		
			PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
			pMeshInstance->addDefaultComponents();

			pMeshInstance->initFromFile(pRealEvent->m_meshFilename, pRealEvent->m_package, pRealEvent->m_threadOwnershipMask);

			
		m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pRealEvent->m_threadOwnershipMask);
		
		// we need to add this mesh to a scene node or to an existing skeleton
		if (pMeshInstance->hasSkinWeights())
		{
			// this mesh has skin weights, so it should belong to a skeleton. assume the last added skeleton is skeleton we need
			PEASSERT(m_lastAddedSkelInstanceHandle.isValid(), "Adding skinned mesh, so we need a skeleton instance");
			m_lastAddedSkelInstanceHandle.getObject<Component>()->addComponent(hMeshInstance);
		}
		else
		{
			if (pRealEvent->hasCustomOrientation)
			{
				// need to create a scene node for this mesh
				Handle hSN("SCENE_NODE", sizeof(SceneNode));
				SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
				pSN->addDefaultComponents();

				pSN->addComponent(hMeshInstance);

				RootSceneNode::Instance()->addComponent(hSN);
				pSN->m_base.setPos(pRealEvent->m_pos);
				pSN->m_base.setU(pRealEvent->m_u);
				pSN->m_base.setV(pRealEvent->m_v);
				pSN->m_base.setN(pRealEvent->m_n);

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//												DOING A THING		Assignment 4								//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				PositionBufferCPU *pvbcpu = pMeshInstance->m_hAsset.getObject<Mesh>()->m_hPositionBufferCPU.getObject<PositionBufferCPU>();

				// Min and Max coordinates for AABB
				PrimitiveTypes::Float32 min[3];
				PrimitiveTypes::Float32 max[3];

				// Initialise
				for (int i = 0; i < 3; i++) {
					min[i] = max[i] = pvbcpu->m_values[i];
				}

				// Get mins and maxes
				for (int i = 3; i < pvbcpu->m_values.m_size; i += 3) {
					for (int j = 0; j < 3; j++) {
						min[j] = min[j] < pvbcpu->m_values[i + j] ? min[j] : pvbcpu->m_values[i + j];
						max[j] = max[j] > pvbcpu->m_values[i + j] ? max[j] : pvbcpu->m_values[i + j];
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

				bool isDynamic = false;

				// Get mesh type
				MeshTypes meshType = normal;
				if (strcmp(pRealEvent->m_meshType, "end") == 0) meshType = end;
				else if (strcmp(pRealEvent->m_meshType, "lava") == 0) meshType = lava;
				else if (strcmp(pRealEvent->m_meshType, "ice") == 0) meshType = ice;
				else if (strcmp(pRealEvent->m_meshType, "spring") == 0) meshType = spring;
				//use pRealEvent->m_meshType to get types. Types as follows:
				/*
					"lava"
					"end"
				*/
				PhysicsComponent *newPhys = new PhysicsComponent(pMeshInstance->min, pMeshInstance->max, base, isDynamic, meshType);
				//my_physics_component = *newPhys; // No need to store here - never updates, and can be accessed from physics manager


				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



				// TESTING FOR PROC GEN
				{
					// need to create a scene node for this mesh
					Handle hSN("SCENE_NODE", sizeof(SceneNode));
					SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
					pSN->addDefaultComponents();

					pSN->addComponent(hMeshInstance);

					RootSceneNode::Instance()->addComponent(hSN);
					pSN->m_base.setPos(Vector3(0.0f,0.0f,0.0f));
					pSN->m_base.setU(pRealEvent->m_u);
					pSN->m_base.setV(pRealEvent->m_v);
					pSN->m_base.setN(pRealEvent->m_n);
				}

			}
			else
			{
				RootSceneNode::Instance()->addComponent(hMeshInstance);
			}
		}

		if (!haveOtherObject)
			m_pContext->getLuaEnvironment()->pushHandleAsFieldAndSet(pRealEvent->m_peuuid, hMeshInstance);
	}
	else
	{

		// already have this object
		// only care about orientation
		if (pRealEvent->hasCustomOrientation)
		{
			// need to reset the orientation
			// try finding scene node
			MeshInstance *pMeshInstance = exisitngObject.getObject<MeshInstance>();
			Handle hSN = pMeshInstance->getFirstParentByType<SceneNode>();
			if (hSN.isValid())
			{
				SceneNode *pSN = hSN.getObject<SceneNode>();
				pSN->m_base.setPos(pRealEvent->m_pos);
				pSN->m_base.setU(pRealEvent->m_u);
				pSN->m_base.setV(pRealEvent->m_v);
				pSN->m_base.setN(pRealEvent->m_n);
			}
		}
	}

	// pop the game object table
	m_pContext->getLuaEnvironment()->pop();
}



void GameObjectManager::do_CONSTRUCT_SOUND(Events::Event *pEvt)
{
	Event_CONSTRUCT_SOUND *pRealEvent = (Event_CONSTRUCT_SOUND *)(pEvt);
	SoundManager::Construct(*m_pContext, m_arena, pRealEvent->m_waveBankFilename);
}
/*
Left for physX reference
void GameObjectManager::do_CREATE_SCENERY(Events::Event *pEvt)
{
	PrimitiveTypes::String modelFilename = "";
	PrimitiveTypes::UInt32 sceneryType = pEvt->getEventData<Events::CreateSceneryEvtData>()->type;
#ifdef PYENGINE_USE_PHYSX
	PhysXComponent::PhysXComponentType physCompType; 
#endif
	switch (sceneryType)
	{
	case SCENERY_CAR:
		modelFilename = "car_mesh.mesha";
		physCompType = PhysXComponent::PhysXComponentType::CUSTOM;
		break;
	case SCENERY_PLANE:
		modelFilename = "brokenplane_mesh.mesha";
		physCompType = PhysXComponent::PhysXComponentType::CUSTOM;
		break;
	case SCENERY_LAMPPOST:
		modelFilename = "streetlight_mesh.mesha";
		physCompType = PhysXComponent::PhysXComponentType::CUSTOM;
		break;
	case SCENERY_BUILDING:
		modelFilename = pEvt->getEventData<Events::CreateSceneryEvtData>()->highResModelName;
		physCompType = PhysXComponent::PhysXComponentType::CUSTOM;
		break;
	case SCENERY_GROUND:
		modelFilename = pEvt->getEventData<Events::CreateSceneryEvtData>()->highResModelName;
		physCompType = PhysXComponent::PhysXComponentType::HEIGHTFIELD;
		break;
	default:
		modelFilename = "car_mesh.mesha"; //We need to get some bushes or something...
	}

	
	//Collidable Object with certain mesh and custom physRep
	Handle hCO(COLLIDABLEOBJECT, sizeof(CollidableObject));
	CollidableObject *pCO = new(hCO) CollidableObject(sceneryType, physCompType, hCO, RootSceneNode::InstanceHandle(), modelFilename, false, "myAwesomeCO");
	pCO->addDefaultComponents();

	//Should use values from lua....

	SceneNode *pSSN = pCO->getComponent<SceneNode>(PE::Components::IDs::PESUUID_PEComp_SCENE_NODE_t::peuuid());
	
	//Position
	pSSN->m_base.setPos(pEvt->getEventData<Events::CreateSceneryEvtData>()->pos); 
	//Orientation
	if(pEvt->getEventData<Events::CreateSceneryEvtData>()->hasCustomOrientation)
	{
		pSSN->m_base.setU(pEvt->getEventData<Events::CreateSceneryEvtData>()->u); 
		pSSN->m_base.setV(pEvt->getEventData<Events::CreateSceneryEvtData>()->v); 
		pSSN->m_base.setN(pEvt->getEventData<Events::CreateSceneryEvtData>()->n); 
	}

	PrimitiveTypes::Float32 physDimX = 0.0f;
	PrimitiveTypes::Float32 physDimY = 0.0f;
	PrimitiveTypes::Float32 physDimZ = 0.0f;
	NxCollisionGroup physCollGroup = Physics::PHYS_GROUP_STATIC_ENVIRONMENT;

	//Custom Physics
	if(pEvt->getEventData<Events::CreateSceneryEvtData>()->hasCustomPhysRep)
	{
		physDimX = pEvt->getEventData<Events::CreateSceneryEvtData>()->physDimX;
		physDimY = pEvt->getEventData<Events::CreateSceneryEvtData>()->physDimY;
		physDimZ = pEvt->getEventData<Events::CreateSceneryEvtData>()->physDimZ;
		physCollGroup = pEvt->getEventData<Events::CreateSceneryEvtData>()->physCollGroup;
	}

	if(sceneryType != SCENERY_GROUND) {
		pCO->instantiatePhysics(physDimX, physDimY, physDimZ, physCollGroup); //dimX, dimY, dimZ, collGroup of custom physrep	
	}
	else
	{
		//height field
		pCO->instantiatePhysics(-2000, 2000, -2000, 2000, 200, "arcaneground.x_param_type_scenery_param_origmodel_arkaneground_param_look_ground__shape1_mesh.mesha"); //startX, endX, startZ, endZ, scale, mesh read in for heights
	}

	GameObjectManager::Instance()->add(hCO);
}
*/

void GameObjectManager::createGameObjectTableIfDoesntExist()
{
	if (m_luaGameObjectTableRef == LUA_NOREF)
		m_luaGameObjectTableRef = LuaEnvironment::createTableOnTopOfStackAndStoreReference(m_pContext->getLuaEnvironment()->L);

}
void GameObjectManager::putGameObjectTableIOnStack()
{
	if (m_luaGameObjectTableRef == LUA_NOREF)
		createGameObjectTableIfDoesntExist();
	
	LuaEnvironment::putTableOnTopOfStackByReference(m_luaGameObjectTableRef, m_pContext->getLuaEnvironment()->L);
}

	
}; // namespace Components
}; //namespace PE

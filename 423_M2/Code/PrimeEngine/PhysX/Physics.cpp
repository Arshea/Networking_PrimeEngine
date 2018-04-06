// Physics.cpp
#define NOMINMAX

// Inter-Engine Includes
#include "../GameObjectModel/GameObjectManager.h"

// Sibling/Children Includes
#include "Physics.h"

#ifdef PYENGINE_USE_PHYSX

namespace PE {
namespace Components {

using namespace PE::Events;

// Physics SDK Singletons
NxPhysicsSDK* Physics::s_pPhysicsSDK = NULL;
NxControllerManager* Physics::s_pManager = NULL;
NxScene* Physics::s_pScene;

// Global Parameters
NxVec3 Physics::gDefaultGravity = NxVec3(0.0f, -98.0f, 0.0f);
NxReal Physics::timeStep = 1.0f/120.0f;
NxReal Physics::stepAccumulator = 0.0f;

// Materials
NxMaterial* Physics::lowFrictionMaterial = NULL;
NxMaterial* Physics::mediumFrictionMaterial = NULL;
NxMaterial* Physics::highFrictionMaterial = NULL;

// Collision Reports and Events
ContactCallback contactReport;
Events::Event s_physCollisionEvt;



void ContactCallback::onContactNotify(NxContactPair& pair, NxU32 events) {
	Physics::ResolveCollisions(pair, events);
}

void Physics::Initialize() {
	s_pPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
	assert(Physics::s_pPhysicsSDK);
	// Connect to Visual Debugger if Available on localhost
	s_pPhysicsSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost");
	
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = gDefaultGravity;
	sceneDesc.simType = NX_SIMULATION_HW;

	// Create the Physics Scene
	Physics::s_pScene = Physics::s_pPhysicsSDK->createScene(sceneDesc);
	if (!Physics::s_pScene)
	{
		sceneDesc.simType = NX_SIMULATION_SW;
		Physics::s_pScene = Physics::s_pPhysicsSDK->createScene(sceneDesc);
	}
	assert(s_pScene);

	s_physCollisionEvt = PEEvt_PHYSICS_COLLISION_t::createStaticEvent();

	// Set Groups we want Collision Callbacks From
	s_pScene->setUserContactReport(&contactReport);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_PLAYER, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_MECH, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_ENEMY, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_PROJECTILE, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_MELEE_WEAPON, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_MECH, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_ENEMY, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_PROJECTILE, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_MELEE_WEAPON, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_ENEMY, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_PROJECTILE, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_MELEE_WEAPON, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_PROJECTILE, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_MELEE_WEAPON, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_MELEE_WEAPON, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_NOTIFY_ON_START_TOUCH);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_STATIC_ENVIRONMENT, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_STATIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_IGNORE_PAIR);
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_STATIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, NX_IGNORE_PAIR); 
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	s_pScene->setActorGroupPairFlags(Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, NX_IGNORE_PAIR);
	
	// Set Groups we want to collide in the physics engine	
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_PLAYER, false);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_MECH, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_ENEMY, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_PROJECTILE, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_MELEE_WEAPON, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PLAYER, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, false);

	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_MECH, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_ENEMY, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_PROJECTILE, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_MELEE_WEAPON, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MECH, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, false);

	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_ENEMY, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_PROJECTILE, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_MELEE_WEAPON, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_ENEMY, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, false);

	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_PROJECTILE, false);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_MELEE_WEAPON, false);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_PROJECTILE, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, false);
	
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_MELEE_WEAPON, false);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, false);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_MELEE_WEAPON, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, false);
	
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_STATIC_ENVIRONMENT, Physics::PHYS_GROUP_STATIC_ENVIRONMENT, false);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_STATIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_STATIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, true);

	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, true);
	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, true);

	s_pScene->setGroupCollisionFlag(Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE, false);

	// Character Controller Manager
	NxUserAllocatorDefault* allocator;
	allocator = new NxUserAllocatorDefault;
	Physics::s_pManager = NxCreateControllerManager(allocator);

	// Set up some default materials
	NxMaterialDesc material;
	material.restitution     = 0.0f;
	material.staticFriction  = 0.1f;
	material.dynamicFriction = 0.1f;
	lowFrictionMaterial = s_pScene->createMaterial(material);

	material.restitution     = 0.0f;
	material.staticFriction  = 50.0f;
	material.dynamicFriction = 1.0f;
	mediumFrictionMaterial = s_pScene->createMaterial(material);

	material.restitution     = 0.0f;
	material.staticFriction  = 100.0f;
	material.dynamicFriction = 1.0f;
	highFrictionMaterial = s_pScene->createMaterial(material);
}

void Physics::LoadDefaults() {
	s_pPhysicsSDK->setParameter(NX_CONTINUOUS_CD, 1);
	s_pPhysicsSDK->setParameter(NX_SKIN_WIDTH, 2.0);
	s_pScene->setGravity(Physics::gDefaultGravity);

	// Default Material
	NxMaterial* defaultMaterial = Physics::s_pScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(50.0f);
	defaultMaterial->setDynamicFriction(1.0f);
}
/*
void Physics::LoadScene() {
	NXU::NxuPhysicsCollection *c = NXU::loadCollection("../AssetsOut/PhysX/city.xml", NXU::FT_XML);
	assert(c);
	NxMat34 instanceFrame;
	instanceFrame.t = NxVec3(0, 0, 0);
	NXU::instantiateCollection(c, *s_pPhysicsSDK, s_pScene, &instanceFrame, NULL);
	
	// Set Collision Group and User Data for loaded Scene thus far (assuming that ONLY the level has been instantiated thus far...)
	NxActor** actors = s_pScene->getActors();
	NxU32 nbActors = s_pScene->getNbActors();
	while (nbActors--) 
	{
		NxU32 nbShapes = actors[nbActors]->getNbShapes();
		actors[nbActors]->setGroup(Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
		while (nbShapes--)
		{
			actors[nbActors]->getShapes()[nbShapes]->setGroup(Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
			actors[nbActors]->getShapes()[nbShapes]->setMaterial(Physics::highFrictionMaterial->getMaterialIndex());
		}
	}
}
*/

void Physics::ReleaseNx() {
	
	if (Physics::s_pScene) Physics::s_pPhysicsSDK->releaseScene(*Physics::s_pScene);
	if (Physics::s_pPhysicsSDK) Physics::s_pPhysicsSDK->release();
}


void Physics::Update(NxReal deltaTime) {
	stepAccumulator += deltaTime;

	while (stepAccumulator >= timeStep) {
		s_pScene->simulate(timeStep);
		s_pScene->flushStream();
		s_pScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		stepAccumulator -= timeStep;
	}

}

bool Physics::GetResults() {
	return true;
	//return s_pScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
}

void Physics::Matrix4x4toNxMat34(Matrix4x4* in, NxMat34* out) {
	out->t = NxVec3(in->getPos().m_x, in->getPos().m_y, -in->getPos().m_z);
	Quaternion q = in->createQuat();
	out->M.fromQuat(NxQuat(NxVec3(q.m_x, q.m_y, q.m_z), q.m_w));
}

void Physics::UpdateKinematics() {
	NxActor** kinematic_actors = s_pScene->getActors();
	NxU32 nbActors = s_pScene->getNbActors();
	
	while (nbActors--)
	{
		if (kinematic_actors[nbActors]->getGroup() == Physics::PHYS_GROUP_MELEE_WEAPON) 
		{
			if (kinematic_actors[nbActors]->userData) {
				SceneNode *pSceneNode = ((PhysXUserData*)(kinematic_actors[nbActors]->userData))->m_hSceneNode.getObject<SceneNode>();
				NxMat34 pose;

				// after evt CALCULATE_TRANSFORMATIONS all the scene nodes have updated m_worldTransform which is its global orientation
				Physics::Matrix4x4toNxMat34(&pSceneNode->m_worldTransform, &pose);
				kinematic_actors[nbActors]->setGlobalPose(pose);
			}
		}
	}
}

void Physics::UpdateControllers() {
	NxU32 nbControllers = Physics::s_pManager->getNbControllers();
	while (nbControllers--)
	{
#ifdef PYENGINE_USE_PHYSX
		((PhysXUserData*)(Physics::s_pManager->getController(nbControllers)->getActor()->userData))->m_hPhysXComponent.getObject<PhysXComponent>()->updateControllerPosition();
#endif
	}
}

PrimitiveTypes::Int32 explosionScalar = 1000;

void Physics::UpdateForceFields(NxReal deltaTime) {
	NxU32 nbForceFields = Physics::s_pScene->getNbForceFields();
	while (nbForceFields--)
	{
		NxForceField* ff = Physics::s_pScene->getForceFields()[nbForceFields];
		ForceFieldUserData* userData = (ForceFieldUserData*)ff->userData;

		if (userData->m_type == Physics::PHYS_FF_EXPLOSION) {
			PrimitiveTypes::Float32 excludeDelta = userData->m_duration / 10;
			if (userData->m_time_active == 0.0f)
				ff->addShapeGroup(*userData->m_inclusionGroup);
			userData->m_inclusionShape->isSphere()->setRadius(userData->m_time_active * explosionScalar * userData->m_magnitude);
			if (userData->m_time_active > excludeDelta)
				userData->m_exclusionShape->isSphere()->setRadius((userData->m_time_active - excludeDelta) * explosionScalar * userData->m_magnitude);
		}

		userData->m_time_active += deltaTime;

		if (userData->m_time_active >= userData->m_duration)
		{
			if(userData->m_type == Physics::PHYS_FF_VORTEX)
			{
				Event veEvt = PEEvt_VORTEX_ENDED_t::createStaticEvent();
				GameObjectManager::Instance()->handleEvent(&veEvt);
				veEvt.releaseEventData();
			}

			Physics::s_pScene->releaseForceField(*ff); // This must be the last call of this function
		}
	}
}

void Physics::ResolveCollisions(NxContactPair& pair, NxU32 events) {
	Events::Event &physCollisionEvent = s_physCollisionEvt;
	Events::PhysicsCollisionData *pCollisionData = physCollisionEvent.getEventData<Events::PhysicsCollisionData>();
	if (pair.actors[0]->userData) pCollisionData->m_node1 = ((PhysXUserData*)(pair.actors[0]->userData))->m_hObject;
	//else pCollisionData->m_node1 = NULL;
	if (pair.actors[1]->userData) pCollisionData->m_node2 = ((PhysXUserData*)(pair.actors[1]->userData))->m_hObject;
	//else pCollisionData->m_node2 = NULL;
	pCollisionData->group1 = (Physics::CollisionGroup)(pair.actors[0]->getGroup());
	pCollisionData->group2 = (Physics::CollisionGroup)(pair.actors[1]->getGroup());
	GameObjectManager::Instance()->handleEvent(&physCollisionEvent);
}

//Sends a simple raycast against the level environment
//Will return two Vector3's : the point of impact ([0]) and the normal ([1])
RaycastHitReport Physics::RaycastEnvironment(Vector3 origin, Vector3 direction, PrimitiveTypes::Int32 physGroupToCollideWith, PrimitiveTypes::Float32 maxDist) {
	NxU32 collideMask = 1 << physGroupToCollideWith;
	NxRay worldRay;
	NxRaycastHit report;
	worldRay.orig = NxVec3(origin.m_x, origin.m_y, -origin.m_z);
	worldRay.dir  = NxVec3(direction.m_x, direction.m_y, -direction.m_z);
	worldRay.dir.normalize();
	Physics::s_pScene->raycastClosestShape(worldRay, NX_ALL_SHAPES, report, collideMask, maxDist, NX_RAYCAST_IMPACT || NX_RAYCAST_NORMAL);  
	RaycastHitReport hit;
	hit.intersection.m_x = report.worldImpact.x;
	hit.intersection.m_y = report.worldImpact.y;
	hit.intersection.m_z = -report.worldImpact.z;
	hit.normal.m_x = report.worldNormal.x;
	hit.normal.m_y = report.worldNormal.y;
	hit.normal.m_z = -report.worldNormal.z; 
	hit.distance = report.distance;
	return hit;
}

void Physics::CreateVortex(Vector3 pos, PrimitiveTypes::Float32 duration, PrimitiveTypes::Float32 magnitude = 1.0f) {
	Handle hUserData(FORCE_FIELD_USER_DATA, sizeof(ForceFieldUserData));
	ForceFieldUserData* pForceFieldUserData = new(hUserData) ForceFieldUserData;
	pForceFieldUserData->m_type = Physics::PHYS_FF_VORTEX;
	pForceFieldUserData->m_duration = duration;
	pForceFieldUserData->m_time_active = 0.0f;
	pForceFieldUserData->m_magnitude = magnitude;
	Physics::CreateVortex(NxVec3(pos.m_x, pos.m_y, -pos.m_z), pForceFieldUserData);
}

void Physics::CreateExplosion(Vector3 pos, PrimitiveTypes::Float32 magnitude = 1.0f) {
	Handle hUserData(FORCE_FIELD_USER_DATA, sizeof(ForceFieldUserData));
	ForceFieldUserData* pForceFieldUserData = new(hUserData) ForceFieldUserData;
	pForceFieldUserData->m_type = Physics::PHYS_FF_EXPLOSION;
	pForceFieldUserData->m_duration = 1.0f;
	pForceFieldUserData->m_time_active = 0.0f;
	pForceFieldUserData->m_magnitude = magnitude;
	Physics::CreateExplosion(NxVec3(pos.m_x, pos.m_y, -pos.m_z), pForceFieldUserData);
}

void Physics::CreateExplosion(NxVec3 pos, ForceFieldUserData* pUserData) {
	NxForceFieldDesc ffDesc;
	NxForceFieldLinearKernelDesc	lKernelDesc;
	NxForceFieldLinearKernel*		linearKernel;

	//constant force of 100 outwards
	lKernelDesc.constant = NxVec3(1500000000 * pUserData->m_magnitude, 0.0f, 0.0f);

	//The forces do not depend on where the objects are positioned
	NxMat33 m;
	m.zero();
	lKernelDesc.positionMultiplier = m;
	lKernelDesc.noise = NxVec3(5,5,5); //adds a random noise on the forces to make the objects a little more chaotic

	//Set target velocity along the radius to 20
	lKernelDesc.velocityTarget = NxVec3(20 * pUserData->m_magnitude,0,0);
	m.diagonal(NxVec3(1,0,0)); //Acts with a force relative to the current velocity to reach the
							   //target velocities. 0 means that those components won't be affected
	lKernelDesc.velocityMultiplier = m;

	// create linear kernel
	linearKernel = s_pScene->createForceFieldLinearKernel(lKernelDesc);
	ffDesc.kernel = linearKernel;
	
	//Attach the force field to an actor (kinematic) so that we can move it around 
	// (spawn the explosions in different places)
	//ffDesc.actor = actor;

	//Create the force field around origo
	NxMat34 pose;
	pose.id();
	pose.t = pos;
	ffDesc.pose = pose;

	ffDesc.coordinates = NX_FFC_SPHERICAL;
	ffDesc.flags = 0;

	NxForceField* forceField = s_pScene->createForceField(ffDesc);
	
	//Attach an include shape, we will animate the size of this later on, so that it grows (like a slow explosion)
	// inclusion group
	NxForceFieldShapeGroupDesc sgInclDesc;
	pUserData->m_inclusionGroup = s_pScene->createForceFieldShapeGroup(sgInclDesc);
	NxForceFieldShape* shape = NULL;
	NxSphereForceFieldShapeDesc s;
	s.radius = 0.1f;
	s.pose.t = pos;
	//s.pose.t = NxVec3(0, 0, 0);
	pUserData->m_inclusionShape = pUserData->m_inclusionGroup->createShape(s);

	// exclusion group
	NxForceFieldShapeGroupDesc sgExclDesc;
	sgExclDesc.flags = NX_FFSG_EXCLUDE_GROUP;
	pUserData->m_exclusionGroup = s_pScene->createForceFieldShapeGroup(sgExclDesc);
	NxSphereForceFieldShapeDesc exclude;
	exclude.radius = 0.2f;
	exclude.pose.t = pos;
	//exclude.pose.t = NxVec3(0, 0, 0);
	pUserData->m_exclusionShape = pUserData->m_exclusionGroup->createShape(exclude);

	forceField->addShapeGroup(*pUserData->m_exclusionGroup);

	forceField->userData = pUserData;
}

void Physics::CreateVortex(NxVec3 pos, ForceFieldUserData* pUserData) {
	NxForceFieldDesc ffDesc;
	NxForceFieldLinearKernelDesc	lKernelDesc;
	NxForceFieldLinearKernel*		linearKernel;

	ffDesc.coordinates = NX_FFC_CYLINDRICAL;
	//Attach the vortex in an actor (which we use for moving the field around in the world)
	//ffDesc.actor = actor;
	//attach the force field at the center of the actor
	NxMat34 pose;
	pose.id();
	pose.t = pos;
	ffDesc.pose = pose;

	//constant force of 30 towards the center (which is then counter-acted by radial forces specified below)
	//constant force of 4 upwards (creating a constant lift on the objects)
	lKernelDesc.constant = NxVec3(-1500000000 * pUserData->m_magnitude, 90000000.0f * pUserData->m_magnitude, 0); 

	//The target where we want the objects to end up is at radius 3 from the center. We use
	//Y=0 as the target in along the y-axis together with the m(0,1)=-5 to create a force
	//directed outwards from the center of the vortex when objects are floating towards the
	//top of the vortex.
	lKernelDesc.positionTarget = NxVec3(50, 100, 0);
	//lKernelDesc.positionTarget = NxVec3(pos.x + 15, pos.y, pos.z);

	//Setup radial forces, depending on where the objects are positioned
	NxMat33 m;
	m.zero();
	m(0,0) =  1000000 * pUserData->m_magnitude; //radial error -> radial force. If outside of target radius, act with a force of 10*distance inwards
	m(0,1) = -500000 * pUserData->m_magnitude; //axial error -> radial force. If the y component of the object position is above the target y position (0), 
				 //then act with a force of 5*distance outwards. This reduces the force of 30 inwards that we setup earlier,
				 //making the vortex broaden out in the top
	m(0,2) = 0;  //there is no tangential error in cylindrical coordinates, so we just set this to 0
	lKernelDesc.positionMultiplier = m;
	lKernelDesc.noise = NxVec3(0, 0, 0); //adds a random noise on the forces to make the objects a little more chaotic

	//Set target velocity along the tangent of the vortex to 30 (the other directions to 0)
	lKernelDesc.velocityTarget = NxVec3(0,0,3000);
	m.diagonal(NxVec3(300,300,300)); //Acts with a force relative to the current velocity to reach the
							   //target velocities. If the velocity is above 30 in radial direction, then
							   //the radial velocity is decreased. If the velocity is below 30 in tangential
							   //direction, then the velocity is increased until it reaches that velocity.
	lKernelDesc.velocityMultiplier = m;

	//You can try some fall-off forces if you e.g. want the vortex to lose power 
	//along the radial direction when the distance from its center increases:
	//lKernelDesc.falloffLinear = NxVec3(5.0f, 0, 0);
	//lKernelDesc.falloffQuadratic = NxVec3(5.0f, 0, 0);
	linearKernel = s_pScene->createForceFieldLinearKernel(lKernelDesc);
	ffDesc.kernel = linearKernel;
	ffDesc.flags = 0;

	NxForceField *forceField = s_pScene->createForceField(ffDesc);
	forceField->userData = pUserData;

	//Attach an include shape, we position this so that it covers the vortex specified above
	NxBoxForceFieldShapeDesc b;
	b.dimensions = NxVec3(800, 800, 800);
	b.pose.t = NxVec3(0, 400, 0);;
	forceField->getIncludeShapeGroup().createShape(b);

	//Create an exclude shape, positioned around the shed

	/*
	NxForceFieldShapeGroupDesc sgDesc;
	sgDesc.flags = NX_FFSG_EXCLUDE_GROUP;
	m_excludeGroup = Physics::s_pScene->createForceFieldShapeGroup(sgDesc);
	
	NxBoxForceFieldShapeDesc exclude;
	exclude.dimensions = NxVec3(2.25f, 1.5f, 1.75f);
	exclude.pose.t = NxVec3(8.85f, 1.5f, -10.3f);
	m_excludeShape = m_excludeGroup->createShape(exclude);

	gForceField->addShapeGroup(*m_excludeGroup);
	*/
}

//void Physics::CreateHeightField(NxVec3 pos, NxU32 nbColumns, NxU32 nbRows) {
//	NxHeightFieldDesc heightFieldDesc;
//
//	heightFieldDesc.nbColumns           = nbColumns;
//	heightFieldDesc.nbRows              = nbRows;
//	heightFieldDesc.verticalExtent      = -1000;
//	heightFieldDesc.convexEdgeThreshold = 0;
//
//
//	// allocate storage for samples
//	heightFieldDesc.samples             = new NxU32[nbColumns*nbRows];
//	heightFieldDesc.sampleStride        = sizeof(NxU32);
//
//	NxU8* currentByte = (NxU8*)heightFieldDesc.samples;
//
//	for (NxU32 row = 0; row < nbRows; row++)
//		{
//		for (NxU32 column = 0; column < nbColumns; column++)
//			{
//			NxHeightFieldSample* currentSample = (NxHeightFieldSample*)currentByte;
//
//			currentSample->height         = 10;//computeHeight(row,column); //desired height value. Singed 16bit integer
//			//currentSample->materialIndex0 = gMaterial0;
//			//currentSample->materialIndex1 = gMaterial1;
//
//			currentSample->tessFlag = 0;
//
//			currentByte += heightFieldDesc.sampleStride;
//			}
//		}
//
//	NxHeightField* heightField = s_pPhysicsSDK->createHeightField(heightFieldDesc);
//
//	// data has been copied, we can free our buffer
//	delete[] heightFieldDesc.samples;
//
//	NxHeightFieldShapeDesc heightFieldShapeDesc;
//
//	heightFieldShapeDesc.heightField     = heightField;
//	//heightFieldShapeDesc.heightScale     = gVerticalScale;
//	//heightFieldShapeDesc.rowScale        = gHorizontalScale;
//	//heightFieldShapeDesc.columnScale     = gHorizontalScale;
//	heightFieldShapeDesc.materialIndexHighBits = 0;
//	heightFieldShapeDesc.holeMaterial = 2;
//	heightFieldShapeDesc.meshFlags = 0;
//
//	NxActorDesc actorDesc;
//
//	actorDesc.shapes.pushBack(&heightFieldShapeDesc);
//	actorDesc.globalPose.t = pos;
//
//	NxActor* newActor = s_pScene->createActor(actorDesc);
//	
//	//newActor->userData = data;
//	newActor->setGroup(Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
//	//SetShapeCollisionGroups(newActor, Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
//}

}; // namespace Components
}; // namespace PE

#endif // #ifdef PYENGINE_USE_PHYSX

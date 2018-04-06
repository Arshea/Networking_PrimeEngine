// PhysXCompnent.cpp

#include "../GameObjectModel/GameObjectManager.h"

// Sibling/Children Includes
#include "PhysXComponent.h"
#ifdef PYENGINE_USE_PHYSX

namespace PE {
namespace Components {

// Controller Collision Callback disabled since we are using only shape-wise collision

//Events::Event PhysXComponent::s_physCollisionEvt;

/*
NxControllerAction ControllerCallback::onShapeHit(const NxControllerShapeHit& hit) {
	
	if (hit.shape) {
		if (hit.dir.y == 0.0f) { //Only account for horizontal hits to improve solver performance
			//if (hit.shape->getGroup() != Physics::GROUP_COLLIDABLE_NON_PUSHABLE) {
				Events::Event &physCollisionEvent = PhysXComponent::s_physCollisionEvt;
				Events::PhysicsCollisionData *pCollisionData = physCollisionEvent.getEventData<Events::PhysicsCollisionData>();
				pCollisionData->m_node1 = ((PhysXUserData*)(hit.controller->getActor()->userData))->m_hObject;
				if (hit.shape->getActor().userData) pCollisionData->m_node2 = ((PhysXUserData*)(hit.shape->getActor().userData))->m_hObject;
				pCollisionData->pos.m_x = hit.worldPos.x;
				pCollisionData->pos.m_y = hit.worldPos.y;
				pCollisionData->pos.m_z = hit.worldPos.z;
				GameObjects::GameObjectManager::Instance()->handleEvent(&physCollisionEvent);
			//}
		}
	}
	
	return NX_ACTION_NONE;
}

NxControllerAction ControllerCallback::onControllerHit(const NxControllersHit& hit) {
	return NX_ACTION_NONE;
}
*/

PhysXComponent::PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode, Handle hsceneNode):Events::Component(hMyself)  //This line is for custom physrep parameters if applicable
{
	if (!hsceneNode.isValid())
	{
		hsceneNode = getComponent(parentNode, PE::Components::IDs::PESUUID_PEComp_SCENE_NODE_t::peuuid());
	}
	
	disp_Nx = Physics::gDefaultGravity;
	m_controller = NULL;
	m_actor = NULL;
	m_hMyself = hMyself;
	m_type = type;
	m_componentTypePEUUID = PE::Components::IDs::PESUUID_PEComp_PHYSICS_t::peuuid();

	// Set handles in PhysXUserData to parent, scene node, and physXcomponent
	Handle hUserData(PHYSX_USER_DATA, sizeof(PhysXUserData));
	PhysXUserData* pPhysXUserData = new(hUserData) PhysXUserData();
	pPhysXUserData->m_hObject = parentNode;
	pPhysXUserData->m_hSceneNode = hsceneNode;
	pPhysXUserData->m_hPhysXComponent = m_hMyself;
	pPhysXUserData->m_type = type;

	// Get initial position of physXcomponent
	NxMat34 pos;
	SceneNode *pSN = hsceneNode.getObject<SceneNode>();

	Physics::Matrix4x4toNxMat34(&pSN->m_base, &pos);

	PhysXComponent::InstantiateComponent(pPhysXUserData, pos);
}

PhysXComponent::PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode, Handle hsceneNode, 
							   NxI32 startHeightFieldX, NxI32 endHeightFieldX, NxI32 startHeightFieldZ, NxI32 endHeightFieldZ, NxReal heightFieldScale, const char* heightFieldMeshName)
							   :Events::Component(hMyself) 
{
	if (!hsceneNode.isValid())
	{
		hsceneNode = getComponent(parentNode, PE::Components::IDs::PESUUID_PEComp_SCENE_NODE_t::peuuid());
	}
	
	disp_Nx = Physics::gDefaultGravity;
	m_controller = NULL;
	m_actor = NULL;
	m_hMyself = hMyself;
	m_type = type;
	m_componentTypePEUUID = PE::Components::IDs::PESUUID_PEComp_PHYSICS_t::peuuid();

	// Set handles in PhysXUserData to parent, scene node, and physXcomponent
	Handle hUserData(PHYSX_USER_DATA, sizeof(PhysXUserData));
	PhysXUserData* pPhysXUserData = new(hUserData) PhysXUserData();
	pPhysXUserData->m_hObject = parentNode;
	pPhysXUserData->m_hSceneNode = hsceneNode;
	pPhysXUserData->m_hPhysXComponent = m_hMyself;
	pPhysXUserData->m_type = type;

	pPhysXUserData->m_startX = startHeightFieldX;
	pPhysXUserData->m_endX = endHeightFieldX;
	pPhysXUserData->m_startZ = startHeightFieldZ;
	pPhysXUserData->m_endZ = endHeightFieldZ;
	pPhysXUserData->m_scale = heightFieldScale;
	pPhysXUserData->m_meshName = heightFieldMeshName;

	// Get initial position of physXcomponent
	NxMat34 pos;
	SceneNode *pSN = hsceneNode.getObject<SceneNode>();

	Physics::Matrix4x4toNxMat34(&pSN->m_base, &pos);

	PhysXComponent::InstantiateComponent(pPhysXUserData, pos);
}

PhysXComponent::PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode, Handle hsceneNode, 
							   NxReal customDimX, NxReal customDimY,  NxReal customDimZ, NxCollisionGroup customCollGroup) :Events::Component(hMyself)
{

	if (!hsceneNode.isValid())
	{
		hsceneNode = getComponent(parentNode, PE::Components::IDs::PESUUID_PEComp_SCENE_NODE_t::peuuid());
	}
	//if (!s_physCollisionEvt.hasValidData())
		//s_physCollisionEvt = Events::Event(Events::PHYSICS_COLLISION, sizeof(Events::PhysicsCollisionData));

	disp_Nx = Physics::gDefaultGravity;
	m_controller = NULL;
	m_actor = NULL;
	m_hMyself = hMyself;
	m_type = type;
	m_componentTypePEUUID = PE::Components::IDs::PESUUID_PEComp_PHYSICS_t::peuuid();

	// Set handles in PhysXUserData to parent, scene node, and physXcomponent
	Handle hUserData(PHYSX_USER_DATA, sizeof(PhysXUserData));
	PhysXUserData* pPhysXUserData = new(hUserData) PhysXUserData();
	pPhysXUserData->m_hObject = parentNode;
	pPhysXUserData->m_hSceneNode = hsceneNode;
	pPhysXUserData->m_hPhysXComponent = m_hMyself;
	pPhysXUserData->m_type = type;

	pPhysXUserData->m_dimX = customDimX;
	pPhysXUserData->m_dimY = customDimY;
	pPhysXUserData->m_dimZ = customDimZ;
	pPhysXUserData->m_collGroup = customCollGroup;


	// Get initial position of physXcomponent
	NxMat34 pos;
	SceneNode *pSN = hsceneNode.getObject<SceneNode>();

	Physics::Matrix4x4toNxMat34(&pSN->m_base, &pos);

	PhysXComponent::InstantiateComponent(pPhysXUserData, pos);

	//m_eventsCanHandle.add(Events::ADDED_AS_COMPONENT);
}

void PhysXComponent::InstantiateComponent(PhysXUserData* data, NxMat34 pos) {
	switch(m_type) {
		case (PhysXComponent::PLAYER) :
			m_controller = PhysXComponent::CreatePlayerController(pos);
			m_controller->setCollision(true);
			m_actor = m_controller->getActor();
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_PLAYER);
			m_actor->getShapes()[0]->setCCDSkeleton(CreateBoxCCDSkeleton(25.0f, 85.0f, 25.0f));
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_PLAYER);
			break;
		case (PhysXComponent::SOLDIER) :
			m_controller = PhysXComponent::CreateSoldierController(pos);
			m_controller->setCollision(true);
			m_actor = m_controller->getActor();
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_ENEMY);
			m_actor->getShapes()[0]->setCCDSkeleton(CreateBoxCCDSkeleton(25.0f, 85.0f, 25.0f));
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_ENEMY);
			break;
		case (PhysXComponent::CIVILIAN) :
			m_controller = PhysXComponent::CreateCivilianController(pos);
			m_controller->setCollision(true);
			m_actor = m_controller->getActor();
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_ENEMY);
			m_actor->getShapes()[0]->setCCDSkeleton(CreateBoxCCDSkeleton(25.0f, 85.0f, 25.0f));
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_ENEMY);
			break;
		case (PhysXComponent::MECH) :
			m_controller = PhysXComponent::CreateMechController(pos);
			m_controller->setCollision(true);
			m_actor = m_controller->getActor();
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_MECH);
			m_actor->getShapes()[0]->setCCDSkeleton(CreateBoxCCDSkeleton(25.0f, 85.0f, 25.0f));
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_MECH);
			break;
		case (PhysXComponent::TANK) :
			m_controller = PhysXComponent::CreateTankController(pos);
			m_controller->setCollision(true);
			m_actor = m_controller->getActor();
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_ENEMY);
			// CCD?
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_ENEMY);
			break;
		case (PhysXComponent::BOX) :
			m_actor = PhysXComponent::CreateBoxObject(pos);
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT);
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT);
			break;
		case (PhysXComponent::FLOOR) :
			m_actor = PhysXComponent::CreateFloorObject(pos);
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
			break;
		case (PhysXComponent::HEIGHTFIELD) :
			m_actor = PhysXComponent::CreateHeightFieldObject(pos, 
				data->m_startX, data->m_endX, data->m_startZ, data->m_endZ, data->m_scale, data->m_meshName);
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_STATIC_ENVIRONMENT);
			break;
		case (PhysXComponent::CUSTOM) :
			m_actor = PhysXComponent::CreateCustomObject(pos, 
				data->m_dimX, data->m_dimY, data->m_dimZ, data->m_collGroup);
			m_actor->userData = data;
			m_actor->setGroup(data->m_collGroup);
			SetShapeCollisionGroups(m_actor, data->m_collGroup);
			break;
		case (PhysXComponent::BULLET) :
			m_actor = PhysXComponent::CreateBulletObject(pos);
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_PROJECTILE);
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_PROJECTILE);
			break;
		case (PhysXComponent::GRENADE) :
			m_actor = PhysXComponent::CreateGrenadeObject(pos);
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_PROJECTILE);
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_PROJECTILE);
			break;
		case (PhysXComponent::MECH_ARM_WEAPON):
			m_actor = PhysXComponent::CreateMechArmWeapon(pos);
			m_actor->userData = data;
			
			m_actor->setGroup(Physics::PHYS_GROUP_MELEE_WEAPON);
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_MELEE_WEAPON);
			break;
		case (PhysXComponent::DEAD_SOLDIER):
			m_actor = PhysXComponent::CreateDeadSoldierBoxObject(pos);
			m_actor->userData = data;
			m_actor->setGroup(Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE); 
			NxReal mass = m_actor->getMass();
			SetShapeCollisionGroups(m_actor, Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE);
			break;
	}
}

NxController* PhysXComponent::CreatePlayerController(NxMat34 pos) {
	NxCapsuleControllerDesc desc;
	
	y_disp = 143.0f; // y displacement from the mesh origin to the origin of the capsule (which will be at the center)
					 // If this is too small, your object may start penetrating with the floor, which will cause it to fall out of the scene.

	desc.position.x = pos.t.x;
	desc.position.y = pos.t.y + y_disp;
	desc.position.z = pos.t.z;
	
	//desc.callback = &contactReport;
	desc.height = 190.0f;
	desc.radius = 40.0f;
	desc.skinWidth = 2.0f;
	desc.slopeLimit = cosf(NxMath::degToRad(45.0f));
	desc.stepOffset = 0.5f;
	desc.upDirection = NX_Y;
	desc.climbingMode = CLIMB_EASY;

	return Physics::s_pManager->createController(Physics::s_pScene, desc);
}


NxController* PhysXComponent::CreateSoldierController(NxMat34 pos) {
	NxCapsuleControllerDesc desc;
	
	y_disp = 143.0f; // y displacement from the mesh origin to the origin of the capsule (which will be at the center)
					 // If this is too small, your object may start penetrating with the floor, which will cause it to fall out of the scene.

	desc.position.x = pos.t.x;
	desc.position.y = pos.t.y + y_disp;
	desc.position.z = pos.t.z;

	//desc.callback = &contactReport;
	desc.height = 190.0f;
	desc.radius = 40.0f;
	desc.skinWidth = 2.0f;
	desc.slopeLimit = cosf(NxMath::degToRad(45.0f));
	desc.stepOffset = 0.5f;
	desc.upDirection = NX_Y;
	desc.climbingMode = CLIMB_EASY; 
	
	return Physics::s_pManager->createController(Physics::s_pScene, desc);
}

NxController* PhysXComponent::CreateCivilianController(NxMat34 pos) {
	NxCapsuleControllerDesc desc;
	
	y_disp = 143.0f; // y displacement from the mesh origin to the origin of the capsule (which will be at the center)
					 // If this is too small, your object may start penetrating with the floor, which will cause it to fall out of the scene.

	desc.position.x = pos.t.x;
	desc.position.y = pos.t.y + y_disp;
	desc.position.z = pos.t.z;

	//desc.callback = &contactReport;
	desc.height = 190.0f;
	desc.radius = 40.0f;
	desc.skinWidth = 2.0f;
	desc.slopeLimit = cosf(NxMath::degToRad(45.0f));
	desc.stepOffset = 0.5f;
	desc.upDirection = NX_Y;
	desc.climbingMode = CLIMB_EASY;
	
	return Physics::s_pManager->createController(Physics::s_pScene, desc);
}

NxController* PhysXComponent::CreateMechController(NxMat34 pos) {
	NxCapsuleControllerDesc desc;
	
	y_disp = 143.0f; // y displacement from the mesh origin to the origin of the capsule (which will be at the center)
					 // If this is too small, your object may start penetrating with the floor, which will cause it to fall out of the scene.

	desc.position.x = pos.t.x;
	desc.position.y = pos.t.y + y_disp;
	desc.position.z = pos.t.z;

	//desc.callback = &contactReport;
	desc.height = 190.0f;
	desc.radius = 40.0f;
	desc.skinWidth = 2.0f;
	desc.slopeLimit = cosf(NxMath::degToRad(85.0f));
	desc.stepOffset = 0.5f;
	desc.upDirection = NX_Y;
	desc.climbingMode = CLIMB_EASY;
	
	return Physics::s_pManager->createController(Physics::s_pScene, desc);
}

NxController* PhysXComponent::CreateTankController(NxMat34 pos) {
	y_disp = 120.0f;
	
	NxBoxControllerDesc desc;
	
	desc.position.x = pos.t.x;
	desc.position.y = pos.t.y + y_disp;
	desc.position.z = pos.t.z;
	desc.extents.x = 244.0f;
	desc.extents.y = 116.0f;
	desc.extents.z = 436.0f;

	desc.skinWidth = 2.0f;
	desc.skinWidth = 2.0f;
	desc.slopeLimit = cosf(NxMath::degToRad(45.0f));
	desc.stepOffset = 0.5f;
	desc.upDirection = NX_Y;

	return Physics::s_pManager->createController(Physics::s_pScene, desc);
}

NxActor* PhysXComponent::CreateBoxObject(NxMat34 pos) {
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;

	y_disp = 0.0f;

	boxDesc.dimensions.set(20.0, 40.0, 40.0);
	boxDesc.localPose.t = NxVec3(0.0, 0.0, 0.0);
	boxDesc.group = Physics::PHYS_GROUP_DYNAMIC_ENVIRONMENT;
	boxDesc.materialIndex = Physics::highFrictionMaterial->getMaterialIndex();

	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = &bodyDesc;
	actorDesc.density = 15.0f;
	actorDesc.globalPose = pos;
	return Physics::s_pScene->createActor(actorDesc);	
}

NxActor* PhysXComponent::CreateFloorObject(NxMat34 pos) {
	NxActorDesc actorDesc;
	NxBoxShapeDesc boxDesc;

	//y_disp = -200.0f;

	boxDesc.dimensions.set(20000.0, 1.0, 20000.0);
	boxDesc.localPose.t = NxVec3(0.0, 0.0, 0.0);
	boxDesc.group = Physics::PHYS_GROUP_STATIC_ENVIRONMENT;
	boxDesc.materialIndex = Physics::highFrictionMaterial->getMaterialIndex();
	
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = NULL;
	actorDesc.density = 15.0f;
	actorDesc.globalPose = pos;
	return Physics::s_pScene->createActor(actorDesc);	
}

NxActor* PhysXComponent::CreateHeightFieldObject(NxMat34 pos, NxI32 startX, NxI32 endX, NxI32 startZ, NxI32 endZ, NxReal scale, const char* meshName) {
	NxHeightFieldDesc heightFieldDesc;

	
	MeshCPU mcpu;
	mcpu.ReadMesh(meshName);
	PositionBufferCPU *pvb = mcpu.m_hPositionBufferCPU.getObject<PositionBufferCPU>();
	Array<PrimitiveTypes::Float32> &vals = pvb->m_values;
	PrimitiveTypes::UInt32 numV3 = vals.m_size / 3;

	
	// analyze the mesh boundaries
	PrimitiveTypes::Float32 
		minX = PrimitiveTypes::Constants::c_LargeFloat32,
		maxX = -PrimitiveTypes::Constants::c_LargeFloat32,
		minZ = PrimitiveTypes::Constants::c_LargeFloat32,
		maxZ = -PrimitiveTypes::Constants::c_LargeFloat32;

	for (PrimitiveTypes::UInt32 iv = 0; iv < numV3; iv++)
	{
		Vector3 v(vals[iv*3], vals[iv*3+1], vals[iv*3+2]);
		// the value is in real world coords. need to convert it to PhysX
		// flip z
		v.m_z = -v.m_z;
		if (v.m_x < minX) minX = v.m_x;
		if (v.m_x > maxX) maxX = v.m_x;
		if (v.m_z < minZ) minZ = v.m_z;
		if (v.m_z > maxZ) maxZ = v.m_z;
	}

	if (minX < 0)
		minX = -scale * (ceil(-minX/scale));
	else
		minX = scale * (ceil(minX/scale));

	if (maxX < 0)
		maxX = -scale * (ceil(-maxX/scale));
	else
		maxX = scale * (ceil(maxX/scale));

	if (minZ < 0)
		minZ = -scale * (ceil(-minZ/scale));
	else
		minZ = scale * (ceil(minZ/scale));

	if (maxZ < 0)
		maxZ = -scale * (ceil(-maxZ/scale));
	else
		maxZ = scale * (ceil(maxZ/scale));


	startX = minX; endX = maxX;
	startZ = minZ; endZ = maxZ;

	PrimitiveTypes::Float32
		lenX = endX - startX,
		lenZ = endZ - startZ;

	heightFieldDesc.nbColumns           = lenZ/scale + 1;
	heightFieldDesc.nbRows              = fabs(lenX)/scale + 1;
	heightFieldDesc.verticalExtent      = -1000;
	heightFieldDesc.convexEdgeThreshold = 0;


	// allocate storage for samples
	heightFieldDesc.samples             = new NxU32[heightFieldDesc.nbColumns*heightFieldDesc.nbRows];
	heightFieldDesc.sampleStride        = sizeof(NxU32);

	NxU8* currentByte = (NxU8*)heightFieldDesc.samples;

	for (NxU32 row = 0; row < heightFieldDesc.nbRows; row++)
	{
		for (NxU32 column = 0; column < heightFieldDesc.nbColumns; column++)
		{
			NxHeightFieldSample* currentSample = (NxHeightFieldSample*)currentByte;

			currentSample->height         = -200; //sinf(row/2)*100;//computeHeight(row,column); //desired height value. Singed 16bit integer
			//currentSample->materialIndex0 = gMaterial0;
			//currentSample->materialIndex1 = gMaterial1;

			currentSample->tessFlag = 0;

			currentByte += heightFieldDesc.sampleStride;
		}
	}

	for (PrimitiveTypes::UInt32 iv = 0; iv < numV3; iv++)
	{
		Vector3 v(vals[iv*3], vals[iv*3+1], vals[iv*3+2]);
		// the value is in real world coords. need to convert it to PhysX
		// flip z
		v.m_z = -v.m_z;
		// revert offset
		v.m_x -= startX; v.m_z -= startZ;
		// get to integer indices (not scaled)
		PrimitiveTypes::Float32
		//	frow = (v.m_z / lenZ) * heightFieldDesc.nbRows,
		//	fcol = (v.m_x / lenX) * heightFieldDesc.nbColumns;
			fcol = (v.m_z / lenZ) * heightFieldDesc.nbColumns,
			frow = (v.m_x / lenX) * heightFieldDesc.nbRows;
		if (frow < 0) frow = 0;
		if (fcol < 0) fcol = 0;
		PrimitiveTypes::UInt32 row = frow;
		PrimitiveTypes::UInt32 col = fcol;

		if (row >= heightFieldDesc.nbRows) row = heightFieldDesc.nbRows-1;
		if (col >= heightFieldDesc.nbColumns) col = heightFieldDesc.nbColumns-1;
		
		NxU8* currentByte = (NxU8*)heightFieldDesc.samples;
		currentByte += heightFieldDesc.sampleStride * (heightFieldDesc.nbColumns * row + col);
		NxHeightFieldSample* currentSample = (NxHeightFieldSample*)currentByte;

		currentSample->height = floor(v.m_y);

	}


	NxHeightField* heightField = Physics::s_pPhysicsSDK->createHeightField(heightFieldDesc);

	// data has been copied, we can free our buffer
	delete[] heightFieldDesc.samples;

	NxHeightFieldShapeDesc heightFieldShapeDesc;

	heightFieldShapeDesc.heightField     = heightField;
	//heightFieldShapeDesc.heightScale     = gVerticalScale;
	heightFieldShapeDesc.rowScale        = scale;
	heightFieldShapeDesc.columnScale     = scale;
	heightFieldShapeDesc.materialIndexHighBits = 0;
	heightFieldShapeDesc.holeMaterial = 2;
	heightFieldShapeDesc.meshFlags = 0;

	heightFieldShapeDesc.localPose.t.x = pos.t.x + (startX - pos.t.x);
	heightFieldShapeDesc.localPose.t.z = pos.t.z + (startZ - pos.t.z);


	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&heightFieldShapeDesc);
	actorDesc.globalPose.t = pos.t;
	//actorDesc.globalPose = pos;

	return Physics::s_pScene->createActor(actorDesc);
}

NxActor* PhysXComponent::CreateCustomObject(NxMat34 pos, NxReal dimX, NxReal dimY, NxReal dimZ, NxCollisionGroup collGroup) {
	NxActorDesc actorDesc;
	NxBoxShapeDesc boxDesc;

	//y_disp = -200.0f;

	boxDesc.dimensions.set(dimX, dimY, dimZ);
	boxDesc.localPose = pos;
	boxDesc.localPose.t = NxVec3(0.0, 0.0, 0.0);
	boxDesc.group = collGroup;
	boxDesc.materialIndex = Physics::highFrictionMaterial->getMaterialIndex();
	
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = NULL;
	actorDesc.density = 15.0f;
	actorDesc.globalPose = pos;
	return Physics::s_pScene->createActor(actorDesc);	
}

NxActor* PhysXComponent::CreateDeadSoldierBoxObject(NxMat34 pos) {
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;

	//bodyDesc.flags |= NX_BF_FROZEN_ROT;

	y_disp = 14.0f;
	pos.t.y += y_disp;

	boxDesc.dimensions.set(80.0, 20.0, 20.0);
	boxDesc.localPose.t = NxVec3(0.0, 0.0, 0.0);
	boxDesc.group = Physics::PHYS_GROUP_DYNAMIC_NON_COLLIDABLE;
	boxDesc.materialIndex = Physics::highFrictionMaterial->getMaterialIndex();
	
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = &bodyDesc;
	actorDesc.density = 50.0f;
	actorDesc.globalPose = pos;
	return Physics::s_pScene->createActor(actorDesc);	
}

NxActor* PhysXComponent::CreateBulletObject(NxMat34 pos) {
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;

	boxDesc.dimensions.set(5.0f, 5.0f, 5.0f);
	boxDesc.localPose.t = NxVec3(0.0, 0.0, 0.0);
	boxDesc.group = Physics::PHYS_GROUP_PROJECTILE;
	boxDesc.shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD;

	bodyDesc.flags |= NX_BF_FROZEN_ROT;
	bodyDesc.flags |= NX_BF_DISABLE_GRAVITY;

	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.0f;
	
	actorDesc.globalPose = pos;
	
	return Physics::s_pScene->createActor(actorDesc);
}

NxActor* PhysXComponent::CreateGrenadeObject(NxMat34 pos) {
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxCapsuleShapeDesc capsuleDesc;

	//bodyDesc.flags |= (NX_BF_FROZEN_ROT || NX_BF_DISABLE_GRAVITY);

	capsuleDesc.height = 8.0f;
	capsuleDesc.radius = 5.0f;
	capsuleDesc.localPose.M.rotX(PrimitiveTypes::Constants::c_Pi_F32/2);
	capsuleDesc.localPose.t = NxVec3(0.0f, 0.0f, 0.0f);
	capsuleDesc.group = Physics::PHYS_GROUP_PROJECTILE;
	
	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.0f;
	actorDesc.globalPose.t	= pos.t;
	
	return Physics::s_pScene->createActor(actorDesc);
}

NxActor* PhysXComponent::CreateMechArmWeapon(NxMat34 pos) {
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxCapsuleShapeDesc capsuleDesc;

	capsuleDesc.height = 70.0f;
	capsuleDesc.radius = 12.0f;
	capsuleDesc.localPose.t = NxVec3(0.0f, 0.0f, 0.0f);
	capsuleDesc.localPose.M.rotX(0.0f);
	capsuleDesc.localPose.M.rotY(0.0f);
	capsuleDesc.localPose.M.rotZ(90.0f);
	capsuleDesc.group = Physics::PHYS_GROUP_MELEE_WEAPON;

	//bodyDesc.flags |= NX_BF_KINEMATIC;
	bodyDesc.flags |= NX_BF_DISABLE_GRAVITY;

	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.0f;
	actorDesc.globalPose = pos;

	return Physics::s_pScene->createActor(actorDesc);
}

void PhysXComponent::freezeComponentRotation(PrimitiveTypes::Bool enable) {
	if (enable) {
		m_actor->raiseBodyFlag(NX_BF_FROZEN_ROT);
	} else {
		m_actor->clearBodyFlag(NX_BF_FROZEN_ROT);
	}
}

// Get componant status

Vector3 PhysXComponent::getGlobalPosition() {
	Vector3 t(0, 0, 0);
	t.m_x = m_actor->getGlobalPosition().x;
	t.m_y = m_actor->getGlobalPosition().y - y_disp;
	t.m_z = -(m_actor->getGlobalPosition().z);
	return t;
}

Vector3 PhysXComponent::getPointVelocity() {
	Vector3 t(0.0, 0.0, 0.0);
	t.m_x = m_actor->getPointVelocity(m_actor->getGlobalPosition()).x;
	t.m_y = m_actor->getPointVelocity(m_actor->getGlobalPosition()).y;
	t.m_z = -m_actor->getPointVelocity(m_actor->getGlobalPosition()).z;
	return t;
}

// Set component status
void PhysXComponent::moveComponent(Vector3 disp) {
	if (m_controller) {
		disp_Nx.x += disp.m_x;
		//disp_Nx.y += disp.m_y;
		disp_Nx.z -= disp.m_z;
	} else {
		NxVec3 t;
		t.x = disp.m_x + m_actor->getGlobalPosition().x;
		t.y = disp.m_y + m_actor->getGlobalPosition().y;
		t.z = -disp.m_z + m_actor->getGlobalPosition().z;
		m_actor->setGlobalPosition(t);
	}
}

void PhysXComponent::addImpulse(Vector3 imp) {
	NxVec3 i(imp.m_x, imp.m_y, -imp.m_z);
	m_actor->addForce(i, NX_IMPULSE);
	
}

void PhysXComponent::updateControllerPosition() {
	NxU32 collisionFlags;
	m_controller->move(disp_Nx, 1, 0.001f, collisionFlags);
	disp_Nx = Physics::gDefaultGravity;
}

void PhysXComponent::setTranslationMatrix(Matrix4x4 *m) {
	NxF64 n[3][3];
	m_actor->getGlobalPose().M.getColumnMajor(n);
	m->setPos(Vector3(m_actor->getGlobalPose().t[0], m_actor->getGlobalPose().t[1], -m_actor->getGlobalPose().t[2]));
}

void PhysXComponent::setPositionMatrix(Matrix4x4 *m) {
	NxF64 n[3][3];
	m_actor->getGlobalPose().M.getColumnMajor(n);
	m->setU(Vector3((PrimitiveTypes::Float32)(n[0][0]), (PrimitiveTypes::Float32)(n[0][1]), -(PrimitiveTypes::Float32)(n[0][2])));
	m->setV(Vector3((PrimitiveTypes::Float32)(n[1][0]), (PrimitiveTypes::Float32)(n[1][1]), -(PrimitiveTypes::Float32)(n[1][2])));
	m->setN(Vector3((PrimitiveTypes::Float32)(n[2][0]), (PrimitiveTypes::Float32)(n[2][1]), -(PrimitiveTypes::Float32)(n[2][2])));
	m->setPos(Vector3(m_actor->getGlobalPose().t[0], m_actor->getGlobalPose().t[1] + y_disp, -m_actor->getGlobalPose().t[2]));
}

void PhysXComponent::ReleaseActors() {
	if (m_controller != NULL) Physics::s_pManager->releaseController(*m_controller);
	if (m_actor != NULL) Physics::s_pScene->releaseActor(*m_actor);
}

void PhysXComponent::Reinitialize() {
	PhysXUserData *pud = (PhysXUserData*)(m_actor->userData);
	ReleaseActors();
	NxMat34 mat_pos;
	SceneNode *pSN = pud->m_hSceneNode.getObject<SceneNode>();
	Physics::Matrix4x4toNxMat34(&pSN->m_base, &mat_pos);
	InstantiateComponent(pud, mat_pos);
}



void PhysXComponent::setKinematicFlagByType(PhysXComponent::PhysXComponentType type, bool enable) {
	// Do not use this function on group PHYS_GROUP_STATIC_ENVIRONMENT, it will not do anything
	
	NxU32 nbActors = Physics::s_pScene->getNbActors();
	while (nbActors--) {
		NxActor* actor = Physics::s_pScene->getActors()[nbActors];
		PhysXUserData* userData = (PhysXUserData*) actor->userData;
		
		if (userData) {
			if (userData->m_type == type) {
				if (enable) {
					actor->raiseBodyFlag(NX_BF_KINEMATIC);
				} else {
					actor->clearBodyFlag(NX_BF_KINEMATIC);
				}
			}
		}
	}
}
		

// Private Methods
void PhysXComponent::SetShapeCollisionGroups(NxActor* actor, NxCollisionGroup group) {
	NxU32 nbShapes = actor->getNbShapes();

	while(nbShapes--) {
		actor->getShapes()[nbShapes]->setGroup(group);
	}
}

NxCCDSkeleton* PhysXComponent::CreateBoxCCDSkeleton(NxReal size_x, NxReal size_y, NxReal size_z) {
	NxU32 triangles[3 * 12] = { 
		0,1,3,
		0,3,2,
		3,7,6,
		3,6,2,
		1,5,7,
		1,7,3,
		4,6,7,
		4,7,5,
		1,0,4,
		5,1,4,
		4,0,2,
		4,2,6
	};

	NxVec3 points[8];

	// Static mesh
	points[0].set( size_x, -size_y, -size_z);
	points[1].set( size_x, -size_y,  size_z);
	points[2].set( size_x,  size_y, -size_z);
	points[3].set( size_x,  size_y,  size_z);

	points[4].set(-size_x, -size_y, -size_z);
	points[5].set(-size_x, -size_y,  size_z);
	points[6].set(-size_x,  size_y, -size_z);
	points[7].set(-size_x,  size_y,  size_z);

	NxSimpleTriangleMesh stm;
	stm.numVertices = 8;
	stm.numTriangles = 6*2;
	stm.pointStrideBytes = sizeof(NxVec3);
	stm.triangleStrideBytes = sizeof(NxU32)*3;

	stm.points = points;
	stm.triangles = triangles;
	stm.flags |= NX_MF_FLIPNORMALS;
	return Physics::s_pPhysicsSDK->createCCDSkeleton(stm);
}

// Component ------------------------------------------------------------
void PhysXComponent::handleEvent(Events::Event *pEvt)
{
	if (Events::PEEvt_MOVE_t::is(pEvt))
	{
		do_MOVE(pEvt);
	}
	else
	{
		Component::handleEvent(pEvt);
	}
}

// Individual events -------------------------------------------------------

void PhysXComponent::do_MOVE(Events::Event *pEvt)
{
	Events::MoveEvtData *pData = pEvt->getEventData<Events::MoveEvtData>();
	this->moveComponent(pData->m_dir);
}

}; // namespace Components
}; // namespace PE

#endif // ifdef PYENGINE_USE_PHYSX

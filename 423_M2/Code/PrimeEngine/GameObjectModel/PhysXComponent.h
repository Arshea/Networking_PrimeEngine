#ifndef __PYENGINE_2_0_PHYSXCOMPONENT_H__
#define __PYENGINE_2_0_PHYSXCOMPONENT_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "../PhysX/Physics.h"
#include "../Math/Vector3.h"
#include "../Scene/SceneNode.h"

#ifdef PYENGINE_USE_PHYSX
namespace PE {
namespace Components {
/*
class ControllerCallback : public NxUserControllerHitReport
{
	virtual NxControllerAction onShapeHit(const NxControllerShapeHit& hit);
	virtual NxControllerAction onControllerHit(const NxControllersHit& hit);

};
*/

struct PhysXComponent : public Events::Component
{

	enum PhysXComponentType
	{
		PLAYER = 0,
		SOLDIER = 1,
		CIVILIAN = 2,
		MECH = 3,
		BOX = 4,
		BULLET = 5,
		GRENADE = 6,
		MECH_ARM_WEAPON = 7,
		DEAD_SOLDIER = 8,
		TANK = 9,
		FLOOR = 10,
		HEIGHTFIELD = 11,
		CUSTOM = 12,
	};

	NxController* m_controller;
	NxActor* m_actor;
	PrimitiveTypes::Float32 y_disp;
	PhysXComponentType m_type;

	// Constructor ----------------
	//PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type);
	//PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode);
	PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode, Handle sceneNode = Handle());
	PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode, Handle sceneNode,
		NxI32 startHeightFieldX, NxI32 endHeightFieldX, NxI32 startHeightFieldZ, NxI32 endHeightFieldZ, NxReal heightFieldScale, const char* heightFieldMeshName); /*This is for making heightfield physrep*/
	PhysXComponent(Handle hMyself, PhysXComponent::PhysXComponentType type, Handle parentNode, Handle sceneNode,
		NxReal customDimX, NxReal customDimY,  NxReal customDimZ, NxCollisionGroup customCollGroup);  /*This is for making custom physrep*/

	// Componant Contructors
	NxController* CreatePlayerController(NxMat34 pos);
	NxController* CreateSoldierController(NxMat34 pos);
	NxController* CreateCivilianController(NxMat34 pos);
	NxController* CreateMechController(NxMat34 pos);
	NxController* CreateTankController(NxMat34 pos);
	NxActor* CreateDeadSoldierBoxObject(NxMat34 pos);
	NxActor* CreateBoxObject(NxMat34 pos);
	NxActor* CreateBulletObject(NxMat34 pos);
	NxActor* CreateGrenadeObject(NxMat34 pos);
	NxActor* CreateMechArmWeapon(NxMat34 pos);
	NxActor* CreateFloorObject(NxMat34 pos);
	NxActor* CreateHeightFieldObject(NxMat34 pos, NxI32 startX, NxI32 endX, NxI32 startZ, NxI32 endZ, NxReal scale, const char* meshName);
	NxActor* CreateCustomObject(NxMat34 pos, NxReal dimX, NxReal dimY, NxReal dimZ, NxCollisionGroup collGroup);

	//ControllerCallback contactReport;
	
	//Displacement vector for Character Movement
	NxVec3 disp_Nx;

	//Position and Velocity Accessors
	Vector3 getGlobalPosition();
	Vector3 getPointVelocity();
	
	//Group Changes
	static void setKinematicFlagByType(PhysXComponent::PhysXComponentType type, bool enable);

	//Component Movement Functions
	void setPositionMatrix(Matrix4x4 *m);
	void setTranslationMatrix(Matrix4x4 *m);
	void moveComponent(Vector3 disp);
	void addImpulse(Vector3 imp);

	// Re-Initialization and Release
	void Reinitialize();
	void ReleaseActors();

	void freezeComponentRotation(PrimitiveTypes::Bool enable);

	// Manipulation
	void setGravity(PrimitiveTypes::Bool enable)
	{
		enable ? m_actor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY) : m_actor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	}

	//Update Functions
	void updateControllerPosition();

	// Component ------------------------------------------------------------
	virtual void handleEvent(Events::Event *pEvt);
	
	// Individual events -------------------------------------------------------
	virtual void do_MOVE(Events::Event *pEvt);

	Handle m_hMyself; // handle to itself
	Handle m_hComponentParent;

private:
	static void SetShapeCollisionGroups(NxActor *actor, NxCollisionGroup group);
	NxCCDSkeleton* CreateBoxCCDSkeleton(NxReal size_x, NxReal size_y, NxReal size_z);
	void InstantiateComponent(PhysXUserData* data, NxMat34 pos);

};

}; // namespace Components
}; // namespace PE

#endif // ifdef PYENGINE_USE_PHYSX

#endif

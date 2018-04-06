#ifndef __PYENGINE_2_0_PHYSICS_H__ 
#define __PYENGINE_2_0_PHYSICS_H__ 

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

//External Includes
#include <assert.h>

#ifdef PYENGINE_USE_PHYSX


// Inter Engine Includes
//#include "NxuStream2/NXU_helper.h"

// Outer Engine Includes
#include <NxPhysics.h>
#include <NxControllerManager.h>
#include <NxController.h>
#include <NxUserAllocatorDefault.h>
#include <NxBoxController.h>
#include <NxCapsuleController.h>
#include <NxUserRaycastReport.h>

namespace PE {
namespace Components {

struct RaycastHitReport
{
	Vector3 intersection;
	Vector3 normal;
	PrimitiveTypes::Float32 distance;
};

// This struct is passed as user data
struct PhysXUserData : PE::PEAllocatableAndDefragmentable
{
	Handle m_hPhysXComponent; // Handle to the PhysXComponent
	Handle m_hSceneNode;      // Handle to the Scene Node that the PhysXComponent is attached to
	Handle m_hObject;		  // Handle to the parent game object
	PrimitiveTypes::Int32 m_type;

	//For heightfield
	NxReal m_scale;
	NxU32 m_startX;
	NxU32 m_endX;
	NxU32 m_startZ;
	NxU32 m_endZ;

	//For custom physcomponent
	NxReal m_dimX;
	NxReal m_dimY;
	NxReal m_dimZ;
	NxCollisionGroup m_collGroup;
	const char* m_meshName;
};

struct ForceFieldUserData : PE::PEAllocatableAndDefragmentable
{
	PrimitiveTypes::UInt32 m_type;
	PrimitiveTypes::Float32 m_time_active;
	PrimitiveTypes::Float32 m_duration;
	PrimitiveTypes::Float32 m_magnitude;

	NxForceFieldShapeGroup* m_inclusionGroup;
	NxForceFieldShapeGroup* m_exclusionGroup;

	NxForceFieldShape* m_inclusionShape;
	NxForceFieldShape* m_exclusionShape;
};

class ContactCallback : public NxUserContactReport 
{
	void onContactNotify(NxContactPair& pair, NxU32 events);
};

class Physics 
{

private:
	//static NxPhysicsSDK* s_pPhysicsSDK;
	//static Events::Event s_physCollisionEvt;
	static void CreateVortex(NxVec3 pos, ForceFieldUserData* pUserData);
	static void CreateExplosion(NxVec3 pos,  ForceFieldUserData* pUserData);

public:

	enum ForceFieldType
	{
		PHYS_FF_VORTEX,
		PHYS_FF_EXPLOSION,
	};

	// Maximum Groups allowed is 31 - Static Environment must be 0
	enum CollisionGroup
	{
		PHYS_GROUP_PLAYER = 1,
		PHYS_GROUP_MECH = 2,
		PHYS_GROUP_ENEMY = 3,
		PHYS_GROUP_PROJECTILE = 4,
		PHYS_GROUP_MELEE_WEAPON = 5,
		PHYS_GROUP_DYNAMIC_ENVIRONMENT = 6,
		PHYS_GROUP_DYNAMIC_NON_COLLIDABLE = 7,
		PHYS_GROUP_STATIC_ENVIRONMENT = 0,
	};

	static NxMaterial* lowFrictionMaterial;
	static NxMaterial* mediumFrictionMaterial;
	static NxMaterial* highFrictionMaterial;

	static NxVec3 gDefaultGravity;
	static NxReal timeStep;
	static NxReal stepAccumulator;

	static void UpdateControllers();
	static void UpdateKinematics();
	static void UpdateForceFields(NxReal deltaTime);

	static NxPhysicsSDK* s_pPhysicsSDK;
	static NxScene* s_pScene;
	static NxControllerManager* s_pManager;
	static void Initialize();
	//static void LoadScene();
	static void LoadDefaults();
	static void ReleaseNx();
	static void Update(NxReal deltaTime);
	static bool GetResults();
	static void ResolveCollisions(NxContactPair& pair, NxU32 events);
	static RaycastHitReport RaycastEnvironment(Vector3 origin, Vector3 direction, PrimitiveTypes::Int32 physGroupToCollideWith = Physics::PHYS_GROUP_STATIC_ENVIRONMENT, PrimitiveTypes::Float32 maxDist = NX_MAX_F32);
	static void CreateVortex(Vector3 pos, PrimitiveTypes::Float32 duration, PrimitiveTypes::Float32 magnitude);
	static void CreateExplosion(Vector3 pos, PrimitiveTypes::Float32 magnitude);
	//static void CreateHeightField(NxVec3 pos, NxU32 nbColumns, NxU32 nbRows);
	static void Matrix4x4toNxMat34(Matrix4x4* in, NxMat34* out);
};

}; // namespace Components
}; // namespace PE

#endif //#ifdef PYENGINE_USE_PHYSX

#endif

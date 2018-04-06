#ifndef __PYENGINE_2_0_PHYSICSCOMPONENT__
#define __PYENGINE_2_0_PHYSICSCOMPONENT__

#include "Shape.h"
#include "PrimeEngine/Events/Component.h"


struct PhysicsComponent 
{
	bool isDynamic;
	Box colliderBox;
	Sphere colliderSphere;
	Vector3 nextPosition;
	float gravityAcceleration = 0.12f; // v/s added when not grounded
	float jumpVelocity = 0.65f; // Velocity of jump
	Vector3 currentVelocity = Vector3(0.0f, 0.0f, 0.0f); // Record of current velocity for rigid body acceleration
	// NOT dependent on frame time !!
	float speed = 8.0f;
	float verticalTerminalVelocity[2] = { -0.95f, 0.95f }; // Cannot fall faster or jump faster than this at any point
	float maxHInputSpeed = 0.75f; // Cannot possess horizontal velocity -component from input- faster than this
	float defaultControl = 1.0f, airControl = 0.01f, iceControl = 0.02f; // Amount of control depending on situation

	MeshTypes state = none;

	// Could have rb controller (accelerations for movement instead of velocities) with drag (& snap to 0) and max velocity (as with terminal vel for vertical)
	// Make sure to use m_frameTime when applying to velocities
	// This way, could maintain velocity upon jumping, and maybe have limited control while in the air, as well as apply changeable drag

	PhysicsComponent() {}
	PhysicsComponent(PrimitiveTypes::Float32 min[3], PrimitiveTypes::Float32 max[3], Matrix4x4 &_base, bool _isDynamic, MeshTypes _meshType);

	void resetPos(Vector3 _pos);
	MeshTypes isGrounded();
	void resetGravitySpeed();
	void incrementGravitySpeed(PrimitiveTypes::Float32 m_frameTime);
	void limitVelocity();
	void jump();
	void springJump();
	Vector3 getCurrentVelocity();
	void updateVelocity(Vector3 _move);
	void showDebug();
	void showDebugSphere();
	void showDebugBox();
	void preUpdate(PrimitiveTypes::Float32 m_frameTime);
	void update();
	Vector3 postUpdate(); // return new position



};

namespace PE {


	namespace Events {
		struct Event_PHYSICS_PRE_UPDATE : public PE::Events::Event {
			PE_DECLARE_CLASS(Event_PHYSICS_PRE_UPDATE);
			Event_PHYSICS_PRE_UPDATE(Vector3 curPos, Vector3 targetPos, PhysicsComponent &physics, PrimitiveTypes::Float32 m_frameTime);
		};
	}
}

#endif
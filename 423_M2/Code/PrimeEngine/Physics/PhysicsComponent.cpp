#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "PhysicsComponent.h"

#include "PhysicsManager.h"

// Frame Time
#include "PrimeEngine/Events/StandardGameEvents.h"


// Assignment 4 Debug renderer
#include "PrimeEngine/Scene/DebugRenderer.h"

namespace PE {

	// Events sent by behavior state machine (or other high level state machines)
	// these are events that specify where a soldier should move
	namespace Events {

		PE_IMPLEMENT_CLASS1(Event_PHYSICS_PRE_UPDATE, Event);

		Event_PHYSICS_PRE_UPDATE::Event_PHYSICS_PRE_UPDATE(Vector3 _curPos, Vector3 _targetMove, PhysicsComponent &myPhysics, PrimitiveTypes::Float32 m_frameTime)
		{
			myPhysics.resetPos(_curPos + Vector3(0.0f, myPhysics.colliderSphere.radius, 0.0f)); // Don't need to do every frame really - only when pos is forced reset on death. Make an event for this separately.

			// Here handle if already clipping (push player upwards until not clipping)
			//myPhysics.handleClipping();

			// Check if grounded
			myPhysics.state = myPhysics.isGrounded();

			if (myPhysics.state != none) { // If touching floor

				if (myPhysics.state == spring) {
					myPhysics.springJump(); // reflects any negative vertical velocity
				}
				else {
					myPhysics.resetGravitySpeed(); // stop gravity acceleration
				}

				// Did they try to jump?
				if (_targetMove.m_y > 0.0f) { // That's the flag. 1.0 attempted move up.
					myPhysics.jump();
				} // Can overwrite spring jump if larger (holding jump gets you to max jump velocity)

			}
			else { // If in air
				myPhysics.incrementGravitySpeed(m_frameTime);
			}

			// Set move from input movement and rb physics etc
			myPhysics.updateVelocity(_targetMove);

			myPhysics.limitVelocity();

			// update intended position
			myPhysics.preUpdate(m_frameTime);

			// Check col with statics & update next position
			myPhysics.update();

			// Commit changes
			myPhysics.colliderSphere.commit();

			myPhysics.showDebug();


		}
	}
}

PhysicsComponent::PhysicsComponent(PrimitiveTypes::Float32 min[3], PrimitiveTypes::Float32 max[3], Matrix4x4 &_base, bool _isDynamic, MeshTypes _meshType) {
	isDynamic = _isDynamic;
	if (isDynamic) {
		Sphere *new_col = new Sphere(min, max, _base);
		colliderSphere = *new_col;

		physicsManager.addToDynamicArray(*this);
	}
	else {
		Box *new_col = new Box(min, max, _base, _meshType);
		colliderBox = *new_col;

		physicsManager.addToStaticArray(*this);
		showDebug();
	}
}

void PhysicsComponent::resetPos(Vector3 _pos) {
	colliderSphere.resetPos(_pos);
}

MeshTypes PhysicsComponent::isGrounded() {
	return physicsManager.isGrounded(*this);
}

void PhysicsComponent::resetGravitySpeed() {
	if (currentVelocity.m_y <= 0.0f) currentVelocity.m_y = 0.0f;
	// Only if landing
}

void PhysicsComponent::incrementGravitySpeed(PrimitiveTypes::Float32 m_frameTime) {
	currentVelocity.m_y -= (gravityAcceleration * m_frameTime * 10.0f);
}

void PhysicsComponent::limitVelocity() {
	// Y Component
	if (currentVelocity.m_y < verticalTerminalVelocity[0]) currentVelocity.m_y = verticalTerminalVelocity[0];
	else if (currentVelocity.m_y > verticalTerminalVelocity[1]) currentVelocity.m_y = verticalTerminalVelocity[1];

	// X, Z Components (Should be done before adding velocity from moving platforms!)
	// This should limit velocity from input ONLY
	if (currentVelocity.m_x < -maxHInputSpeed) currentVelocity.m_x = -maxHInputSpeed;
	else if (currentVelocity.m_x > maxHInputSpeed) currentVelocity.m_x = maxHInputSpeed;
	if (currentVelocity.m_z < -maxHInputSpeed) currentVelocity.m_z = -maxHInputSpeed;
	else if (currentVelocity.m_z > maxHInputSpeed) currentVelocity.m_z = maxHInputSpeed;

}

void PhysicsComponent::jump() {
	if (currentVelocity.m_y < jumpVelocity) currentVelocity.m_y = jumpVelocity;
}

void PhysicsComponent::springJump() {
	if (currentVelocity.m_y < 0.0f) {
		currentVelocity.m_y = -currentVelocity.m_y;
	}
}

Vector3 PhysicsComponent::getCurrentVelocity() {
	return currentVelocity;
}

void PhysicsComponent::updateVelocity(Vector3 _move) {

	float control = 1.0f;

	switch (state) {
	case normal:
		control = defaultControl;
		break;
	case ice:
		control = iceControl;
		break;
	case none:
		control = airControl;
		break;
	case lava: // Remove all veclocity
		control = 1.0f;
		_move = Vector3(0.0f, 0.0f, 0.0f);
		break;
	case spring:
		control = airControl;
		break;
	default:
		control = defaultControl;
		break;
	}

	currentVelocity.m_x = (1 - control) * getCurrentVelocity().m_x + control * _move.m_x;
	currentVelocity.m_z = (1 - control) * getCurrentVelocity().m_z + control * _move.m_z;
}

void PhysicsComponent::preUpdate(PrimitiveTypes::Float32 m_frameTime) {
	Vector3 frameMove = currentVelocity * m_frameTime * speed;

	colliderSphere.preUpdate(frameMove);
}

void PhysicsComponent::update() {
	physicsManager.update(*this);
}

Vector3 PhysicsComponent::postUpdate() {
	return colliderSphere.centre;
}

void PhysicsComponent::showDebug() {
	if (isDynamic) showDebugSphere();
	else showDebugBox();

}

void PhysicsComponent::showDebugSphere() {

	Vector3 color = { 1.0f, 1.0f, 1.0f };
	Vector3 toDebug[60];
	for (int i = 0; i < 60; i += 2) {
		toDebug[i] = colliderSphere.centre;
		toDebug[i + 1] = color;
	}

	// Offsets for edge vertices

	// Axes to edges
	toDebug[0].m_x += colliderSphere.radius;
	toDebug[2].m_x -= colliderSphere.radius;
	toDebug[4].m_y += colliderSphere.radius;
	toDebug[6].m_y -= colliderSphere.radius;
	toDebug[8].m_z += colliderSphere.radius;
	toDebug[10].m_z -= colliderSphere.radius;

	// Build shape
	toDebug[12].m_x -= colliderSphere.radius;
	toDebug[14].m_y -= colliderSphere.radius;
	toDebug[16].m_y -= colliderSphere.radius;
	toDebug[18].m_z -= colliderSphere.radius;
	toDebug[20].m_z -= colliderSphere.radius;
	toDebug[22].m_x -= colliderSphere.radius;

	toDebug[24].m_x -= colliderSphere.radius;
	toDebug[26].m_y += colliderSphere.radius;
	toDebug[28].m_y -= colliderSphere.radius;
	toDebug[30].m_z += colliderSphere.radius;
	toDebug[32].m_z -= colliderSphere.radius;
	toDebug[34].m_x += colliderSphere.radius;

	toDebug[36].m_x += colliderSphere.radius;
	toDebug[38].m_y -= colliderSphere.radius;
	toDebug[40].m_y += colliderSphere.radius;
	toDebug[42].m_z -= colliderSphere.radius;
	toDebug[44].m_z += colliderSphere.radius;
	toDebug[46].m_x -= colliderSphere.radius;

	toDebug[48].m_x += colliderSphere.radius;
	toDebug[50].m_y += colliderSphere.radius;
	toDebug[52].m_y += colliderSphere.radius;
	toDebug[54].m_z += colliderSphere.radius;
	toDebug[56].m_z += colliderSphere.radius;
	toDebug[58].m_x += colliderSphere.radius;

	PE::Components::DebugRenderer::Instance()->createLineMesh(true, colliderBox.base, &toDebug[12].m_x, 24, 1);
}

void PhysicsComponent::showDebugBox() {
	Vector3 color = { 0.0f, 0.0f, 0.0f };
	Vector3 linepts[48];

	// Array list of vertices to include.
	int verticesToAdd[] = { 0,1,0,2,0,4,3,1,3,2,3,7,5,1,5,4,5,7,6,2,6,4,6,7 };
	for (int i = 0; i < 24; i++) {
		linepts[2 * i] = colliderBox.vertices[verticesToAdd[i]];
		linepts[2 * i + 1] = color;
	}
	PE::Components::DebugRenderer::Instance()->createLineMesh(true, colliderBox.base, &linepts[0].m_x, 24, 1000000);

}


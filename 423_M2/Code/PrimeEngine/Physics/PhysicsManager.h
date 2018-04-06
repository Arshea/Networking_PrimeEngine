#ifndef __PYENGINE_2_0_PHYSICSMANAGER__
#define __PYENGINE_2_0_PHYSICSMANAGER__

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Utils/PEClassDecl.h"
#include "PhysicsComponent.h"

struct PhysicsManager
{
	PhysicsComponent statics[100];
	int num_statics = 0;
	PhysicsComponent dynamics[100];
	int num_dynamics = 0;

	void addToDynamicArray(PhysicsComponent &toAdd) {
		dynamics[num_dynamics++] = toAdd;
		char buf[100];
		sprintf(buf, "Number of dynamic elements: %d\n", num_dynamics);
		OutputDebugStringA(buf);
	}

	void addToStaticArray(PhysicsComponent &toAdd) {
		statics[num_statics++] = toAdd;
		char buf[100];
		sprintf(buf, "Number of static elements: %d\n", num_statics);
		OutputDebugStringA(buf);
	}

	MeshTypes isGrounded(PhysicsComponent &dynComp) {
		//float give = 0.1f; // 0.1 either way counts as touching -- need to playtest......
		for (int i = 0; i < num_statics; i++) {
			if (statics[i].colliderBox.max[1] <= dynComp.colliderSphere.centre.getY() // Else the static isn't below us - no need to check
				&& statics[i].colliderBox.max[0] >= (dynComp.colliderSphere.centre.getX() - dynComp.colliderSphere.radius)
				&& statics[i].colliderBox.max[2] >= (dynComp.colliderSphere.centre.getZ() - dynComp.colliderSphere.radius)
				&& statics[i].colliderBox.min[0] <= (dynComp.colliderSphere.centre.getX() + dynComp.colliderSphere.radius)
				&& statics[i].colliderBox.min[2] <= (dynComp.colliderSphere.centre.getZ() + dynComp.colliderSphere.radius)) // Else dynComp is not directly over statComp
			{
				Vector3 toAabb = collision(dynComp, statics[i]);
				float distanceSqr = toAabb.lengthSqr();
				float radSqr = dynComp.colliderSphere.radius * dynComp.colliderSphere.radius; // Sqr is quicker but accurate for <1.0?
				if (distanceSqr < radSqr && toAabb.m_y * toAabb.m_y == distanceSqr) { // Clipping vertically! Snap upwards??? Otherwise jump can be registered on consecutive frames
					dynComp.colliderSphere.centre.m_y += (dynComp.colliderSphere.radius - fabs(toAabb.m_y));
				}
				if (distanceSqr <= radSqr) { // If contact with floor
					return statics[i].colliderBox.meshType;
				}
			}

		}
		return none; // No mesh there
	}

	void update(PhysicsComponent &dynComp) {
		for (int i = 0; i < num_statics; i++) {
			Vector3 toAabb = collision(dynComp, statics[i]);
			float distanceSqr = toAabb.lengthSqr();
			float radSqr = dynComp.colliderSphere.radius * dynComp.colliderSphere.radius; // Sqr is quicker but accurate for <1.0?
			if (distanceSqr <= radSqr) {

				Vector3 intendedMove = dynComp.colliderSphere.next - dynComp.colliderSphere.centre;

				// If direction of movement is towards AABB
				if ((toAabb.m_x < 0 && intendedMove.m_x < 0) || (toAabb.m_x > 0 && intendedMove.m_x > 0)) {
					// If movement component in this direction is determining factor (furthest from AABB closest point in this direction)
					if (fabs(toAabb.m_x) > fabs(toAabb.m_y) && fabs(toAabb.m_x) > fabs(toAabb.m_z)) {
						dynComp.colliderSphere.next.m_x = dynComp.colliderSphere.centre.m_x;
						//dynComp.colliderSphere.next.m_x = (1 - ratio[0]) * dynComp.colliderSphere.next.m_x + (ratio[0]) * dynComp.colliderSphere.centre.m_x; // ratio alternative
					}
				}
				if ((toAabb.m_y < 0 && intendedMove.m_y < 0) || (toAabb.m_y > 0 && intendedMove.m_y > 0)) {
					if (fabs(toAabb.m_y) > fabs(toAabb.m_z) && fabs(toAabb.m_y) > fabs(toAabb.m_x)) {
						dynComp.colliderSphere.next.m_y = dynComp.colliderSphere.centre.m_y;
						//dynComp.colliderSphere.next.m_y = (1 - ratio[1]) * dynComp.colliderSphere.next.m_y + (ratio[1]) * dynComp.colliderSphere.centre.m_y; // ratio alternative
					}
				}
				if ((toAabb.m_z < 0 && intendedMove.m_z < 0) || (toAabb.m_z > 0 && intendedMove.m_z > 0)) {
					if (fabs(toAabb.m_z) > fabs(toAabb.m_y) && fabs(toAabb.m_z) > fabs(toAabb.m_x)) {
						dynComp.colliderSphere.next.m_z = dynComp.colliderSphere.centre.m_z;
						//dynComp.colliderSphere.next.m_z = (1 - ratio[2]) * dynComp.colliderSphere.next.m_z + (ratio[2]) * dynComp.colliderSphere.centre.m_z; // ratio alternative
					}
				}


			}
		}
	}

	Vector3 collision(PhysicsComponent &dynComp, PhysicsComponent &statComp) {
		Vector3 s = dynComp.colliderSphere.centre;
		Vector3 bMin = statComp.colliderBox.vertices[0];
		Vector3 bMax = statComp.colliderBox.vertices[7];

		Vector3 closestP = closestPoint(s, bMin, bMax);

		Vector3 toClosest = closestP - s;

		return toClosest;
	}

	// Find closest point on AABB to sphere centre
	Vector3 closestPoint(Vector3 s, Vector3 bMin, Vector3 bMax) {
		Vector3 closest;
		closest.m_x = clamp(s.m_x, bMin.m_x, bMax.m_x);
		closest.m_y = clamp(s.m_y, bMin.m_y, bMax.m_y);
		closest.m_z = clamp(s.m_z, bMin.m_z, bMax.m_z);
		return closest;
	}

	// Find closest point for a single axis
	float clamp(float dynVal, float statMin, float statMax) {
		if (dynVal < statMin) return statMin;
		else if (dynVal > statMax) return statMax;
		else return dynVal;
	}
};

PhysicsManager physicsManager;




#endif


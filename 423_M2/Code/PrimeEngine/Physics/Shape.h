#ifndef __PYENGINE_2_0_SHAPE__
#define __PYENGINE_2_0_SHAPE__
#include "PrimeEngine/Math/Vector3.h"
#include "PrimeEngine/Math/Matrix4x4.h"

enum MeshTypes { none = 0, normal, lava, end, ice, spring };


struct Box {


	Matrix4x4 base;
	PrimitiveTypes::Float32 min[3], max[3];
	Vector3 vertices[8];
	MeshTypes meshType = normal;

	Box()  {}

	Box(PrimitiveTypes::Float32 _min[3], PrimitiveTypes::Float32 _max[3], Matrix4x4 &_base, MeshTypes _meshType) {

		Vector3 *toAdd = new Vector3(_min[0], _min[1], _min[2]);
		vertices[0] = *toAdd;
		toAdd = new Vector3(_max[0], _min[1], _min[2]);
		vertices[1] = *toAdd;
		toAdd = new Vector3(_min[0], _max[1], _min[2]);
		vertices[2] = *toAdd;
		toAdd = new Vector3(_max[0], _max[1], _min[2]);
		vertices[3] = *toAdd;
		toAdd = new Vector3(_min[0], _min[1], _max[2]);
		vertices[4] = *toAdd;
		toAdd = new Vector3(_max[0], _min[1], _max[2]);
		vertices[5] = *toAdd;
		toAdd = new Vector3(_min[0], _max[1], _max[2]);
		vertices[6] = *toAdd;
		toAdd = new Vector3(_max[0], _max[1], _max[2]);
		vertices[7] = *toAdd;

		// Added for raycast simplicity
		for (int i = 0; i < 3; i++) {
			min[i] = _min[i];
			max[i] = _max[i];
		}
		base = _base;

		meshType = _meshType;
	}


};

struct Sphere {
	float radius;
	Vector3 centre;
	Vector3 next;
	float gravityInit = 0.08F; // Try falling
	float gravityAccel = 1.05F; // Rate of acceleration
	float gravityBase = -500.0F; // Don't fall further than this
	Sphere() {}

	Sphere(PrimitiveTypes::Float32 min[3], PrimitiveTypes::Float32 max[3], Matrix4x4 &_base) {
		float centreX, centreY, centreZ;
		centreX = (min[0] + max[0]) / 2;
		centreY = (min[1] + max[1]) / 2;
		centreZ = (min[2] + max[2]) / 2;
		Vector3 *centreP = new Vector3(centreX, centreY, centreZ);
		centre = *centreP;

		// Calc max radius
		// Assume character furthest point is dependent on y axis only (not unreasonable, and helps with gravity accuracy via collision with ground)
		radius = (max[1] - min[1]) / 2;
	}

	void resetPos(Vector3 _pos) {
		centre = _pos;
	}

	void preUpdate(Vector3 move) {
		next = centre + move;
	}

	void update(Vector3 forbidMove) {
		next -= forbidMove;
	}

	// Do I need this?
	void commit() {
		centre = next;
	}
};

#endif
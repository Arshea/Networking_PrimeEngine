#ifndef __PYENGINE_2_0_PARTICLE_H__
#define __PYENGINE_2_0_PARTICLE_H__

#include "PrimeEngine/Math/Vector3.h"

struct Particle {
	Vector3 position = Vector3(0, 0, 0);
	Vector3 velocity = Vector3(0, 0, 0);
	int life = 10;
	float size = 1.0f;
	float rotation = 0.0f, rotationRate = 0.0f;
	float initialSize = 1.0f; int initialLife = 10; Vector3 initialPosition = Vector3(0, 0, 0); // Required for lerping calculations
	Vector3 drift = Vector3(0, 0, 0); // Sideways drift of fire
	int texCoords[2] = { 0,0 };	//Row and column for texture

	Particle::Particle() {}

	Particle::Particle(Vector3 _initialOffset, Vector3 _velocity, int _initialLife, float _initialSize, float _initialRotation, float _rotationRate, int _texCoords[2], Vector3 pDrift) {
		initialSize = _initialSize;
		initialLife = _initialLife;
		initialPosition = _initialOffset;

		position = _initialOffset;
		velocity = _velocity;
		life = _initialLife;
		size = _initialSize;
		rotation = _initialRotation;
		rotationRate = _rotationRate;
		drift = pDrift;

		texCoords[0] = _texCoords[0];
		texCoords[1] = _texCoords[1];
	}

	// Return false if dead
	bool update() {
		life--;
		if (life <= 0) return false;
		return true;
	}
};

#endif
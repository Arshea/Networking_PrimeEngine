#ifndef __PYENGINE_2_0_PARTICLESYSTEM_H__
#define __PYENGINE_2_0_PARTICLESYSTEM_H__

#include "PrimeEngine/Math/Vector3.h"
#include "PrimeEngine/Scene/Particle.h"
#include "PrimeEngine/Scene/TextureProperties.h"

struct PresetPSystParams {
	TextureList tex = FIRE_1;
	int numParticles = 500;
	bool looping = true;
	int initialLife[2] = { 20,60 };
	float initialSize[2] = { 1.0f,1.0f }, initialSpeed[2] = { 15.0f,15.0f }, gravityF = 0.0f, endSize = 0.0f, emissionRadius = 0.0f,
		initialPRotation[2] = { 0.0f, 0.0f }, pRotationRate[2] = { 0.0f, 0.0f };
	Vector3 initialDirection[2] = { Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f) },
		initialOffset = Vector3(0.0f, 0.0f, 0.0f), endPosition = Vector3(0.0f, 0.0f, 0.0f);
	bool sizeOverLifetime = false, initialDirectionRotate = false, headingTowardsPosition = false, pRotate = false;
	int systemLife = 500; // Lifetime of system
	bool raycast = true; float systemDistFromCamera = 5.0f;// Raycast onto ground? Else pick a distance from camera to place
	bool fireDrift = false /*small random move for fire*/, bubbleDrift = false, ghostDrift = false, bounce = false /*on floor*/, fixedToCam = false /* moves with camera*/; // System specific controls
	bool orientateWithVelocity = false;
	bool deathSubsystem = false, hasTrail = false, isTrail = false, isSubsystem = false; // SUBSYSTEMS
	int trailOfIndex = 0, trailPreset = 0, deathSubsystemPreset = 0;
	bool killOnCollision = false;
};

struct ParticleSystem {
	TextureList tex = FIRE_1;
	Vector3 basePosition = Vector3(0, 0, 0);
	Particle particles[1000]; // transformed particles for rendering
	int numParticles = 500, numParticlesLeft = 500;
	bool looping = true;
	int initialLife[2] = { 20,60 };
	float initialSize[2] = { 1.0f,1.0f }, initialSpeed[2] = { 15.0f,15.0f }, gravityF = 0.0f, endSize = 0.0f, emissionRadius = 0.0f,
		initialPRotation[2] = { 0.0f, 0.0f }, pRotationRate[2] = { 0.0f, 0.0f };
	Vector3 initialDirection[2] = { Vector3(-0.3f, 1.0f, -0.3f), Vector3(-0.3f, 1.0f, -0.3f) },
		initialOffset = Vector3(0.0f, 0.0f, 0.0f), endPosition = Vector3(0.0f, 0.0f, 0.0f);
	bool sizeOverLifetime = false, initialDirectionRotate = false, headingTowardsPosition = false, pRotate = false;
	int maxSystemLife = 500, systemLife = 500; // Lifetime of system	
	bool fireDrift = false /*small random move for fire*/, bubbleDrift = false, ghostDrift = false, bounce = false /*on floor*/, fixedToCam = false /* moves with camera*/; // System specific controls
	bool orientateWithVelocity = false;
	bool deathSubsystem = false /* explode on death */, hasTrail = false, isTrail = false, isSubsystem = false; // SUBSYSTEMS
	int trailOfIndex = 0, trailPreset = 0, deathSubsystemPreset = 0;
	bool killOnCollision = false;

	bool readyToGo = false; // Only true once initialisation has finished


	ParticleSystem::ParticleSystem(PresetPSystParams preset, Vector3 raycastPos) {
		basePosition = raycastPos;
		setParameters(preset);
		// Add loop for max num particles
		setUpSystem();
	}

	ParticleSystem::ParticleSystem() {
		// Add loop for max num particles
		setUpSystem();
	}

	void setParameters(PresetPSystParams preset) {
		for (int i = 0; i < 2; i++) {
			initialLife[i] = preset.initialLife[i];
			initialSize[i] = preset.initialSize[i];
			initialSpeed[i] = preset.initialSpeed[i];
			initialDirection[i] = preset.initialDirection[i];
			initialPRotation[i] = preset.initialPRotation[i];
			pRotationRate[i] = preset.pRotationRate[i];
		}
		emissionRadius = preset.emissionRadius;
		gravityF = preset.gravityF;
		endSize = preset.endSize;
		pRotate = preset.pRotate;
		initialOffset = preset.initialOffset;
		headingTowardsPosition = preset.headingTowardsPosition;
		endPosition = preset.endPosition;
		sizeOverLifetime = preset.sizeOverLifetime;
		initialDirectionRotate = preset.initialDirectionRotate;
		deathSubsystem = preset.deathSubsystem;
		deathSubsystemPreset = preset.deathSubsystemPreset;
		isSubsystem = preset.isSubsystem;
		systemLife = preset.systemLife;
		maxSystemLife = systemLife;
		tex = preset.tex;
		fireDrift = preset.fireDrift;
		bounce = preset.bounce;
		fixedToCam = preset.fixedToCam;
		numParticles = preset.numParticles;
		looping = preset.looping;
		orientateWithVelocity = preset.orientateWithVelocity;
		ghostDrift = preset.ghostDrift;
		hasTrail = preset.hasTrail;
		trailPreset = preset.trailPreset;
		isTrail = preset.isTrail;
		trailOfIndex = preset.trailOfIndex;
		numParticlesLeft = numParticles;
		killOnCollision = preset.killOnCollision;
	}

	void setUpSystem() {
		int numParticles = 2;// change later ofc
		for (int i = 0; i < numParticles; i++) {
			particles[i] = makeNewParticle();
		}
	}

	// Dynamically change start properties of particles
	void updateInitialDirection() {
		// Rotate about y axis (will add more options later?? maybe input line to rotate around)
		float angleOffset = 0.05f; // Angle to offset each frame on xz plane
		for (int i = 0; i < 2; i++) {
			// Find radius on xz plane
			float radSqr = initialDirection[i].getX() * initialDirection[i].getX() + initialDirection[i].getZ() * initialDirection[i].getZ();
			float radius = sqrt(radSqr);
			// Move to new direction (maintaining radius)
			float newX = /*radius[i] * */(initialDirection[i].getX() * cos(angleOffset) - initialDirection[i].getZ() * sin(angleOffset)); // Yay vectors
			float newZ = /*radius[i] * */(initialDirection[i].getX() * sin(angleOffset) + initialDirection[i].getZ() * cos(angleOffset));
			initialDirection[i] = Vector3(newX, initialDirection[i].getY(), newZ);
		}
	}

	Particle makeNewParticle() {
		Vector3 p_InitialDirection = randomVector(initialDirection[0], initialDirection[1]);
		p_InitialDirection.normalize();
		Vector3 p_InitialOffset = initialOffset;
		if (emissionRadius > 0.0f) {
			Vector3 p_EmissionPoint = randomVector(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
			p_EmissionPoint.normalize();
			p_EmissionPoint *= emissionRadius;
			p_InitialOffset += p_EmissionPoint;
		}
		float p_InitialSpeed = randomBetweenNumbers(initialSpeed[0], initialSpeed[1]);
		Vector3 p_Velocity = p_InitialSpeed * p_InitialDirection;
		float p_InitialSize = randomBetweenNumbers(initialSize[0], initialSize[1]);
		float p_InitialLife = randomBetweenNumbers(initialLife[0], initialLife[1]);
		float p_InitialRotation = randomBetweenNumbers(initialPRotation[0], initialPRotation[1]);
		float p_rotationRate = randomBetweenNumbers(pRotationRate[0], pRotationRate[1]);
		Vector3 pDrift = Vector3(0, 0, 0);
		if (ghostDrift) pDrift = randomVector(Vector3(-0.01, -0.01, -0.01), Vector3(0.01, 0.01, 0.01));

		int pTexCoords[2] = { randomBetweenNumbers(0, TextureProperties(tex).num_cols), randomBetweenNumbers(0, TextureProperties(tex).num_rows) };

		return Particle(p_InitialOffset, p_Velocity, p_InitialLife, p_InitialSize, p_InitialRotation, p_rotationRate, pTexCoords, pDrift);
	}


	// Update interface -------------------------------------------------------------------------------------------------------------------

	bool update() { // Return false if dead, true else
					// Update system parameters here
		if (initialDirectionRotate) updateInitialDirection();

		for (int i = 0; i < numParticles; i++) {
			// Update particle parameters here

			// Update Size ----------------------------------------------------------------------------------------------------------------
			if (sizeOverLifetime) {
				float progress = 1.0f - ((float)particles[i].life / (float)particles[i].initialLife); // 0 at start; 1 at end
				float newSize = (progress * endSize) + ((1 - progress) * particles[i].initialSize); // Lerp between sizes
				particles[i].size = newSize;
			}

			// These can all be combined of course, and the vectors and mult set in the preset. I just don't have time.
			// Fire drift -----------------------------------------------------------------------------------------------------------------
			if (fireDrift) {
				float driftMultiplier = 0.01; // How strong the effect is
				particles[i].drift += (driftMultiplier * randomVector(Vector3(-1.0f, 0, -1.0f), Vector3(1.0f, 0.1f, 1.0f)));
				particles[i].position += particles[i].drift;
			}
			else if (bubbleDrift) {
				float driftMultiplier = 0.5; // How strong the effect is
				particles[i].drift += (driftMultiplier * randomVector(Vector3(-1.0f, -1.0, -1.0f), Vector3(1.0f, 1.0f, 1.0f)));
				particles[i].position += particles[i].drift;
			}
			else if (ghostDrift) {
				particles[i].velocity += particles[i].drift;

			}

			// Bounce physics -------------------------------------------------------------------------------------------------------------
			if (bounce) {
				if (particles[i].position.getY() < 0.0f) { // INCLUDE OFFSET AND BASE~
					float bounceDampen = 0.8f; // Multiplier to reduce speed on bounce
					particles[i].position.m_y = -particles[i].position.getY(); // Put above plane
					particles[i].velocity.m_y = -particles[i].velocity.getY(); // Flip velocity
					particles[i].velocity *= bounceDampen;
				}
			}

			// Update Velocity and/or Position -------------------------------------------------------------------------------------------
			if (headingTowardsPosition) { // If has a destination
				float progress = 1.0f - ((float)particles[i].life / (float)particles[i].initialLife); // 0 at start; 1 at end
				Vector3 newPos = (progress * endPosition) + ((1 - progress) * particles[i].initialPosition); // Lerp between sizes
				particles[i].position = newPos;
			}
			else { // Else has a velocity
				if (gravityF != 0.0f) particles[i].velocity += Vector3(0, -gravityF, 0); // Add gravity
				particles[i].position += particles[i].velocity;
			}
			if (killOnCollision && particles[i].position.getY() <= 0.0f && particles[i].velocity.getY() <= 0.0f && particles[i].life > 2) {
				particles[i].life = 2;
			}

			// Update rotation ------------------------------------------------------------------------------------------------------------
			if (pRotate) {
				particles[i].rotation += particles[i].rotationRate;
			}
			else if (orientateWithVelocity) {
				Vector3 dir = particles[i].velocity;
				dir.m_z = 0;
				dir.normalize();
				float angle;
				if (dir.getY() > 0.0f) {
					angle = acos(dir.getX()) - (3.1415f / 2.0f);
				}
				else {
					angle = (1.5f * 3.1415f) - acos(dir.getX());
				}

				particles[i].rotation = angle;

			}

			// Update Life ----------------------------------------------------------------------------------------------------------------
			if (!particles[i].update() /*|| /* or if y < 0 -- change later because local only particles[i].position.getY() < 0.0f*/) { // If dead make new
				if (looping || maxSystemLife - systemLife < 10) {
					particles[i] = makeNewParticle();
				}
				else {
					numParticlesLeft--;
					//particles[i].size = 0.0f; // Ok this is bad. Should remove and stop updating this particle. But oh well.
				}
			}
		}
		if (!looping && numParticlesLeft < 1) systemLife = 0; // All particles dead - kill system
		systemLife--;
		if (systemLife < 0) {
			return false;
		}
		return true;
	}

	Particle *getParticles() {
		return particles;
	}


	// Utility -------------------------------------------------------------------------------------------------------------------------------

	Vector3 randomVector(Vector3 min, Vector3 max) {
		Vector3 randomV;
		randomV.m_x = randomBetweenNumbers(min.getX(), max.getX());
		randomV.m_y = randomBetweenNumbers(min.getY(), max.getY());
		randomV.m_z = randomBetweenNumbers(min.getZ(), max.getZ());

		return randomV;
	}

	// Random float between min and max
	float randomBetweenNumbers(float min, float max) {
		if (min == max) {
			return max;
		}
		float random = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		return random;
	}

	// Random int between min and max
	int randomBetweenNumbers(int min, int max) {
		if (min == max) {
			return min;
		}
		int random = min + ((rand()) % (max - min));
		return random;
	}

};

#endif

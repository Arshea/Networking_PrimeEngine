// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "ParticleManager.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Geometry/SkeletonCPU/SkeletonCPU.h"

#include "PrimeEngine/Scene/RootSceneNode.h"

#include "Light.h"

// Sibling/Children includes

#include "MeshInstance.h"
#include "Skeleton.h"
#include "SceneNode.h"
#include "DrawList.h"
#include "SH_DRAW.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"

// Camera stuff
#include "CameraSceneNode.h"
#include "CameraManager.h"



namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(ParticleManager, Component);
		ParticleManager::ParticleManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
			: Component(context, arena, hMyself)
		{
			//pSysTest = new ParticleSystem();
			makePresetSystems();
		}

		void ParticleManager::updateSystems(int &threadOwnershipMask) {
			for (int i = 0; i < maxNumSystems; i++) {
				if (pSysArray[i] != NULL && pSysArray[i]->readyToGo) {
					if (pSysArray[i]->update()) { // alive
						if (pSysArray[i]->fixedToCam) { // Change position according to camera
							CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
							Matrix4x4 camBase = pcam->m_base;
							Vector3 camPos = camBase.getPos();
							Vector3 camDir = camBase.getN();
							float t = 2 * presets[i].systemDistFromCamera;
							pSysArray[i]->basePosition = Vector3(camPos.getX() + t * camDir.getX(), camPos.getY() + t * camDir.getY(), camPos.getZ() + t * camDir.getZ());
						}
						else if (pSysArray[i]->isTrail) { // Is a trail - update position
							if (pSysArray[pSysArray[i]->trailOfIndex] != NULL) {
								// Update position
								pSysArray[i]->initialOffset = pSysArray[pSysArray[i]->trailOfIndex]->particles[0].position;
								// Update direction
								pSysArray[i]->initialDirection[0] = -pSysArray[pSysArray[i]->trailOfIndex]->particles[0].velocity;
								pSysArray[i]->initialDirection[1] = pSysArray[i]->initialDirection[0];
							}
							else pSysArray[i] = NULL; // Parent died
						}
						if (pSysArray[i] != NULL && pSysArray[i]->deathSubsystem && pSysArray[i]->deathSubsystemPreset >= 0 && pSysArray[i]->deathSubsystemPreset < numPresets) {
							for (int j = 0; j < pSysArray[i]->numParticles; j++) {
								if (pSysArray[i]->particles[j].life <= 2) {
									//OutputDebugStringA("Making a subsystem \\o/\n");
									int subSystemIndex = startSystem(pSysArray[i]->deathSubsystemPreset);
									if (subSystemIndex >= 0 && subSystemIndex < maxNumSystems) {
										pSysArray[subSystemIndex]->basePosition = pSysArray[i]->basePosition;
										pSysArray[subSystemIndex]->initialOffset = pSysArray[i]->particles[j].position;
										pSysArray[subSystemIndex]->setUpSystem();
										pSysArray[subSystemIndex]->update();
										pSysArray[subSystemIndex]->readyToGo = true;
									}
								}
							}
						}
						if (pSysArray[i] != NULL && pSysArray[i]->maxSystemLife - pSysArray[i]->systemLife > 10) print(i, threadOwnershipMask); // Let run for a bit first...
					}
					else { // dead
						pSysArray[i] = NULL;
					}
				}
			}
		}

		void ParticleManager::print(int i, int &threadOwnershipMask) {
			if (pSysArray[i] != NULL && pSysArray[i]->readyToGo) {
				if (pSysArray[i]->tex == FIRE_4)
					ParticleRenderer::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				/*else if (pSysArray[i]->tex == BUBBLE)
					ParticleRenderer2::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == SNOWFLAKES)
					ParticleRenderer3::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == FLARE)
					ParticleRenderer4::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == MUSICAL_NOTE)
					ParticleRenderer5::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == BLUE_EXPLOSION)
					ParticleRenderer6::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == KOFFING)
					ParticleRenderer7::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == GHOST)
					ParticleRenderer8::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);
				else if (pSysArray[i]->tex == KOFFING)
					ParticleRenderer9::Instance()->createPSysMesh(
						pSysArray[i]->getParticles(), 0, pSysArray[i]->basePosition, pSysArray[i]->tex, 0.01f, pSysArray[i]->numParticles, threadOwnershipMask);*/
			}
		}

		int ParticleManager::startSystem(int systemPresetID) { // Input key -- start new particle system [RETURNS INDEX OF SYSTEM IN ARRAY IF NEEDED (e.g. for a subsystem)
			int index = findFistUnassignedIndex();
			if (index >= 0 && pSysArray[index] == NULL && systemPresetID < numPresets) {
				// Calculate position of system - kind of raycast
				CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
				Matrix4x4 camBase = pcam->m_base;
				Vector3 camPos = camBase.getPos();
				Vector3 camDir = camBase.getN(); // I think. Forwards. yes
				float t;
				if (presets[systemPresetID].raycast) {
					t = -camPos.getY() / camDir.getY();
				}
				else {
					t = presets[systemPresetID].systemDistFromCamera;
				}
				if (t > 0) { // Else it's behind the camera ]=
					Vector3 raycastPos = Vector3(camPos.getX() + t * camDir.getX(), camPos.getY() + t * camDir.getY(), camPos.getZ() + t * camDir.getZ());
					pSysArray[index] = new ParticleSystem(presets[systemPresetID], raycastPos);
					if (pSysArray[index] != NULL && !pSysArray[index]->isSubsystem) {
						pSysArray[index]->readyToGo = true;
					}

					//if (index == 0) {
					//	// fire -- add small fire as well
					//	int index2 = findFistUnassignedIndex();
					//	if(index2 != -1) pSysArray[index2] = new ParticleSystem(presets[1], raycastPos);
					//}

				}
				if (pSysArray[index] != NULL && pSysArray[index]->hasTrail) {
					int trailIndex = startSystem(pSysArray[index]->trailPreset);
					if (trailIndex >= 0) pSysArray[trailIndex]->trailOfIndex = index;
				}

			}
			return index;
		}

		int ParticleManager::findFistUnassignedIndex() {
			for (int i = 0; i < maxNumSystems; i++) {
				if (pSysArray[i] == NULL) return i;
			}
			return -1;
		}

		void ParticleManager::makePresetSystems() {
			// SHOULD REALLY ENUM FOR CLARITY
			int i;
			// Fire
			{	i = 0;
			presets[i].tex = FIRE_4;
			presets[i].initialLife[0] = 1;
			presets[i].initialLife[1] = 200;
			presets[i].initialSize[0] = 0.3f;
			presets[i].initialSize[1] = 1.2f;
			presets[i].initialSpeed[0] = 0.0f;
			presets[i].initialSpeed[1] = 0.3f;
			presets[i].initialDirection[0] = Vector3(-0.0f, 1.0f, -0.0f);
			presets[i].initialDirection[1] = Vector3(0.0f, 1.0f, 0.0f);
			presets[i].emissionRadius = 3.0f;
			//presets[i].gravityF = 0.35f;
			//presets[i].gravity = true;
			presets[i].endSize = 0.0f;
			presets[i].sizeOverLifetime = true;
			presets[i].systemLife = 500;

			presets[i].numParticles = 1000;

			presets[i].pRotate = true;
			presets[i].initialPRotation[1] = 6.2f;
			presets[i].pRotationRate[0] = -0.05f;
			presets[i].pRotationRate[1] = 0.05f;

			presets[i].fireDrift = true;
			}
			// Smaller Fire
			{	i = 1;
			presets[i].tex = FIRE_4;
			presets[i].initialLife[0] = 1;
			presets[i].initialLife[1] = 200;
			presets[i].initialSize[0] = 0.3f;
			presets[i].initialSize[1] = 0.8f;
			presets[i].initialSpeed[0] = 0.0f;
			presets[i].initialSpeed[1] = 0.05f;
			presets[i].initialDirection[0] = Vector3(-0.1f, 1.0f, -0.1f);
			presets[i].initialDirection[1] = Vector3(0.1f, 1.0f, 0.1f);
			presets[i].emissionRadius = 2.0f;
			//presets[i].gravityF = 0.35f;
			//presets[i].gravity = true;
			presets[i].endSize = 0.0f;
			presets[i].sizeOverLifetime = true;
			presets[i].systemLife = 500;

			presets[i].pRotate = true;
			presets[i].initialPRotation[1] = 6.2f;
			presets[i].pRotationRate[0] = -0.05f;
			presets[i].pRotationRate[1] = 0.05f;

			presets[i].fireDrift = true;
			}
			// Bubbles
			{	i = 2;
			presets[i].tex = BUBBLE;
			presets[i].numParticles = 50;
			presets[i].initialLife[0] = 200;
			presets[i].initialLife[1] = 600;
			presets[i].initialSize[0] = 0.1f;
			presets[i].initialSize[1] = 1.0f;
			presets[i].initialSpeed[0] = 0.0f;
			presets[i].initialSpeed[1] = 0.4f;
			presets[i].initialDirection[0] = Vector3(-0.3f, 1.0f, -0.3f);
			presets[i].initialDirection[1] = Vector3(0.3f, 1.0f, 0.3f);
			presets[i].emissionRadius = 30.0f;
			presets[i].bubbleDrift = true;
			presets[i].endSize = 0.0f;
			//presets[i].sizeOverLifetime = true;
			presets[i].systemLife = 2000;
			presets[i].raycast = false;
			}
			// Spiral
			{	i = 3;
			presets[i].tex = KOFFING;
			presets[i].initialLife[0] = 50;
			presets[i].initialLife[1] = 3000;
			presets[i].initialSize[0] = 1.0f;
			presets[i].initialSize[1] = 1.0f;
			presets[i].initialSpeed[0] = 10.0f;
			presets[i].initialSpeed[1] = 10.0f;
			presets[i].gravityF = 0.105f;
			presets[i].bounce = true;
			//presets[i].endSize = 0.0f;
			//presets[i].sizeOverLifetime = true;
			presets[i].initialDirection[0] = Vector3(-0.5f, 1.0f, -0.5f);
			presets[i].initialDirection[1] = Vector3(-0.5f, 1.0f, -0.5f);
			presets[i].initialDirectionRotate = true;
			presets[i].systemLife = 1500;

			presets[i].pRotate = true;
			presets[i].initialPRotation[1] = 6.2f;
			presets[i].pRotationRate[0] = -0.1f;
			presets[i].pRotationRate[1] = 0.1f;
			}
			// Blue explosion
			{	i = 4;
			presets[i].tex = BLUE_EXPLOSION;
			presets[i].numParticles = 60;
			presets[i].initialLife[0] = 200;
			presets[i].initialLife[1] = 900;
			presets[i].initialSize[0] = 0.0f;
			presets[i].initialSize[1] = 0.0f;
			presets[i].endSize = 2.8f;
			presets[i].sizeOverLifetime = true;
			presets[i].initialSpeed[0] = 0.0f;
			presets[i].initialSpeed[1] = 0.0f;
			presets[i].initialDirection[0] = Vector3(-0.3f, 1.0f, -0.3f);
			presets[i].initialDirection[1] = Vector3(0.3f, 1.0f, 0.3f);
			presets[i].systemLife = 3000;
			presets[i].raycast = false;

			presets[i].pRotate = true;
			presets[i].initialPRotation[1] = 6.2f;
			presets[i].pRotationRate[0] = -0.04f;
			presets[i].pRotationRate[1] = 0.04f;
			}
			// Snow
			{	i = 5;
			presets[i].tex = SNOWFLAKES;
			presets[i].initialLife[0] = 100;
			presets[i].initialLife[1] = 200;
			presets[i].initialSize[0] = 0.05f;
			presets[i].initialSize[1] = 0.35f;
			presets[i].endSize = 0.2f;
			//presets[i].sizeOverLifetime = true;
			presets[i].initialOffset = Vector3(0.0f, 100.0f, 0.0f);
			presets[i].initialDirection[0] = Vector3(-0.2f, -0.8f, -0.2f);
			presets[i].initialDirection[1] = Vector3(-0.0f, -0.8f, -0.2f);
			presets[i].initialSpeed[0] = 1.0f;
			presets[i].initialSpeed[1] = 2.0f;
			presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 800;
			presets[i].raycast = false;

			presets[i].pRotate = true;
			presets[i].initialPRotation[1] = 6.2f;
			presets[i].pRotationRate[0] = -0.06f;
			presets[i].pRotationRate[1] = 0.06f;

			presets[i].systemDistFromCamera = 10.0f;
			presets[i].fixedToCam = true;
			}
			// Ghost
			{	i = 6;
			presets[i].tex = GHOST;
			presets[i].initialLife[0] = 100;
			presets[i].initialLife[1] = 200;
			presets[i].initialSize[0] = 0.05f;
			presets[i].initialSize[1] = 0.75f;
			presets[i].endSize = 0.2f;
			//presets[i].sizeOverLifetime = true;
			presets[i].initialDirection[0] = Vector3(-1.0f, -0.2f, -0.2f);
			presets[i].initialDirection[1] = Vector3(-0.1f, 0.2f, 0.2f);
			presets[i].initialSpeed[0] = 1.0f;
			presets[i].initialSpeed[1] = 2.0f;
			presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 800;
			presets[i].raycast = false;

			presets[i].initialOffset = Vector3(1.0f, 0.0f, 0.0f);

			presets[i].systemDistFromCamera = 10.0f;
			presets[i].fixedToCam = true;

			presets[i].ghostDrift = true;
			presets[i].orientateWithVelocity = true;
			}
			// Ghost from source
			{	i = 7;
			presets[i].tex = GHOST;
			presets[i].initialLife[0] = 100;
			presets[i].initialLife[1] = 500;
			presets[i].initialSize[0] = 0.01f;
			presets[i].initialSize[1] = 0.75f;
			presets[i].initialDirection[0] = Vector3(-0.7f, 0.6f, -0.7f);
			presets[i].initialDirection[1] = Vector3(0.7f, 1.0f, 0.7f);
			presets[i].initialSpeed[0] = 0.4f;
			presets[i].initialSpeed[1] = 1.5f;
			//presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 800;
			presets[i].raycast = false;

			presets[i].numParticles = 600;

			presets[i].systemDistFromCamera = 10.0f;

			presets[i].ghostDrift = true;
			presets[i].orientateWithVelocity = true;
			}
			// single non-loop with trail
			{	i = 8;
			presets[i].tex = FLARE;
			presets[i].initialLife[0] = 100;
			presets[i].initialLife[1] = 500;
			presets[i].initialSize[0] = 1.0f;
			presets[i].initialSize[1] = 1.5f;
			presets[i].initialDirection[0] = Vector3(-0.7f, 0.6f, -0.7f);
			presets[i].initialDirection[1] = Vector3(0.7f, 1.0f, 0.7f);
			presets[i].initialSpeed[0] = 0.9f;
			presets[i].initialSpeed[1] = 1.5f;
			//presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 800;
			presets[i].gravityF = 0.01f;
			presets[i].numParticles = 1;

			presets[i].hasTrail = true;
			presets[i].trailPreset = 9;

			//presets[i].killOnCollision = true;
			presets[i].looping = false;
			}
			// trail
			{	i = 9;
			presets[i].tex = FIRE_4;
			presets[i].initialLife[0] = 2;
			presets[i].initialLife[1] = 40;
			presets[i].initialSize[0] = 0.6f;
			presets[i].initialSize[1] = 2.2f;
			presets[i].endSize = 0.0f;
			presets[i].sizeOverLifetime = true;
			presets[i].initialDirection[0] = Vector3(-0.7f, 0.6f, -0.7f);
			presets[i].initialDirection[1] = Vector3(0.7f, 1.0f, 0.7f);
			presets[i].initialSpeed[0] = 1.0f;
			presets[i].initialSpeed[1] = 1.0f;
			//presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 800;
			presets[i].pRotate = true;
			presets[i].initialPRotation[1] = 6.2f;
			presets[i].pRotationRate[0] = -0.05f;
			presets[i].pRotationRate[1] = 0.05f;
			presets[i].numParticles = 30;

			presets[i].isTrail = true;
			presets[i].trailOfIndex = 0;

			presets[i].looping = true;
			}
			// Exploding bubbles
			{	i = 10;
			presets[i].tex = BUBBLE;
			presets[i].numParticles = 40;
			presets[i].initialLife[0] = 200;
			presets[i].initialLife[1] = 300;
			presets[i].initialSize[0] = 0.0f;
			presets[i].initialSize[1] = 0.1f;
			presets[i].initialSpeed[0] = 0.0f;
			presets[i].initialSpeed[1] = 0.4f;
			presets[i].initialDirection[0] = Vector3(-0.8f, 1.0f, -0.8f);
			presets[i].initialDirection[1] = Vector3(0.8f, 1.0f, 0.8f);
			presets[i].emissionRadius = 20.0f;
			presets[i].endSize = 0.5f;
			presets[i].sizeOverLifetime = true;
			presets[i].systemLife = 2000;
			presets[i].bubbleDrift = true;
			presets[i].raycast = false;
			presets[i].deathSubsystem = true;
			presets[i].deathSubsystemPreset = 11;
			}
			// Bubbles explosion subsystem
			{	i = 11;
			presets[i].tex = BUBBLE;
			presets[i].numParticles = 100;
			presets[i].initialLife[0] = 20;
			presets[i].initialLife[1] = 20;
			presets[i].initialSize[0] = 0.05f;
			presets[i].initialSize[1] = 0.1f;
			presets[i].initialSpeed[0] = 1.0f;
			presets[i].initialSpeed[1] = 1.4f;
			presets[i].initialDirection[0] = Vector3(-1.0f, -1.0f, -1.0f);
			presets[i].initialDirection[1] = Vector3(1.0f, 1.0f, 1.0f);
			presets[i].endSize = 0.0f;
			presets[i].sizeOverLifetime = true;
			presets[i].systemLife = 20;
			presets[i].raycast = false;
			presets[i].isSubsystem = true;
			//presets[i].looping = false;
			}
			// Firework
			{	i = 12;
			presets[i].tex = FLARE;
			presets[i].initialLife[0] = 250;
			presets[i].initialLife[1] = 500;
			presets[i].initialSize[0] = 1.0f;
			presets[i].initialSize[1] = 1.7f;
			presets[i].initialDirection[0] = Vector3(-0.1f, 0.8f, -0.1f);
			presets[i].initialDirection[1] = Vector3(0.1f, 1.0f, 0.1f);
			presets[i].initialSpeed[0] = 8.0f;
			presets[i].initialSpeed[1] = 12.9f;
			//presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 1000;
			presets[i].gravityF = 0.03f;
			presets[i].numParticles = 3;

			presets[i].deathSubsystem = true;
			presets[i].deathSubsystemPreset = 13;

			presets[i].hasTrail = true;
			presets[i].trailPreset = 9;

			presets[i].killOnCollision = true;
			//presets[i].looping = false;
			}
			// "Firework" subsystem
			{	i = 13;
			presets[i].tex = FLARE;
			presets[i].numParticles = 500/*0*/;
			presets[i].initialLife[0] = 50;
			presets[i].initialLife[1] = 450;
			presets[i].initialSize[0] = 0.01f;
			presets[i].initialSize[1] = 0.4f;
			presets[i].initialSpeed[0] = 8.0f;
			presets[i].initialSpeed[1] = 20.0f;
			presets[i].initialDirection[0] = Vector3(-1.0f, -1.0f, -1.0f);
			presets[i].initialDirection[1] = Vector3(1.0f, 1.0f, 1.0f);
			presets[i].endSize = 0.0f;
			//presets[i].sizeOverLifetime = true;
			presets[i].gravityF = 0.03f;
			presets[i].systemLife = 1000;
			presets[i].raycast = false;
			presets[i].isSubsystem = true;
			presets[i].looping = false;
			presets[i].killOnCollision = true;
			//presets[i].bounce = true;

			}
			// Everything - trail, collision death, subsystem on death, gravity, physics
			{	i = 14;
			presets[i].tex = FLARE;
			presets[i].initialLife[0] = 1000;
			presets[i].initialLife[1] = 1000;
			presets[i].initialSize[0] = 1.0f;
			presets[i].initialSize[1] = 1.7f;
			presets[i].initialDirection[0] = Vector3(-0.3f, 0.8f, -0.3f);
			presets[i].initialDirection[1] = Vector3(0.3f, 1.0f, 0.3f);
			presets[i].initialSpeed[0] = 1.2f;
			presets[i].initialSpeed[1] = 2.9f;
			//presets[i].emissionRadius = 300.0f;
			presets[i].systemLife = 2000;
			presets[i].gravityF = 0.02f;
			presets[i].numParticles = 1;

			presets[i].deathSubsystem = true;
			presets[i].deathSubsystemPreset = 15;

			presets[i].hasTrail = true;
			presets[i].trailPreset = 9;

			presets[i].killOnCollision = true;
			presets[i].looping = false;
			}
			// "Everything" subsystem
			{	i = 15;
			presets[i].tex = FLARE;
			presets[i].numParticles = 10/*0*/;
			presets[i].initialLife[0] = 50;
			presets[i].initialLife[1] = 300;
			presets[i].initialSize[0] = 0.2f;
			presets[i].initialSize[1] = 0.5f;
			presets[i].initialSpeed[0] = 2.0f;
			presets[i].initialSpeed[1] = 3.0f;
			presets[i].initialDirection[0] = Vector3(-1.0f, 1.0f, -1.0f);
			presets[i].initialDirection[1] = Vector3(1.0f, 1.0f, 1.0f);
			presets[i].endSize = 0.0f;
			presets[i].sizeOverLifetime = true;
			presets[i].gravityF = 0.03f;
			presets[i].systemLife = 1000;
			presets[i].raycast = false;
			presets[i].isSubsystem = true;
			presets[i].looping = false;
			presets[i].bounce = true;

			}
		}

	}
}
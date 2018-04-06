#ifndef __PYENGINE_2_0_PARTICLERENDERER_H__
#define __PYENGINE_2_0_PARTICLERENDERER_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4x4.h"
#include "SceneNode.h"
#include "ParticleSystem.h"

// Particle system contents


// Sibling/Children includes
namespace PE {
	namespace Components {

		struct ParticleRenderer : public SceneNode
		{
			PE_DECLARE_CLASS(ParticleRenderer);
			// Singleton ------------------------------------------------------------------

			static void Construct(PE::GameContext &context, PE::MemoryArena arena);

			static ParticleRenderer *Instance()
			{
				return s_myHandle.getObject<ParticleRenderer>();
			}

			static Handle InstanceHandle()
			{
				return s_myHandle;
			}

			static void SetInstanceHandle(const Handle &handle)
			{
				// Singleton
				ParticleRenderer::s_myHandle = handle;
			}

			// Constructor -------------------------------------------------------------
			ParticleRenderer(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);
			virtual ~ParticleRenderer() {}
			// Methods      ------------------------------------------------------------

			void createPSysMesh(const Particle *pSyst, float timeToLive, Vector3 pos, TextureList tex, float scale, int numParticles,
				int &threadOwnershipMask);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
			virtual void do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt);

			void postPreDraw(int &threadOwnershipMask);
			// Component ------------------------------------------------------------

			virtual void addDefaultComponents();
			// Individual events -------------------------------------------------------



		private:
			static Handle s_myHandle;
			Handle m_hMyPSysMesh;
			static const int NUM_PSysSceneNodes = 64;
			Handle m_hSNPool[NUM_PSysSceneNodes];
			int m_hFreeingSNs[NUM_PSysSceneNodes];
			int m_hAvailableSNs[NUM_PSysSceneNodes];
			float m_lifetimes[NUM_PSysSceneNodes];
			int m_numAvaialble;
			int m_numFreeing;

		};

	}; // namespace Components
}; // namespace PE
#endif

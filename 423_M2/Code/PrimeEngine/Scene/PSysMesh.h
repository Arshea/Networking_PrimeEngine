#ifndef __PYENGINE_2_0_PSYSMESH_H__
#define __PYENGINE_2_0_PSYSMESH_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

// Sibling/Children includes
#include "Mesh.h"

#include "ParticleSystem.h"

namespace PE {
	namespace Components {


		struct PSysMesh : public Mesh
		{
			PE_DECLARE_CLASS(PSysMesh);

			// Constructor -------------------------------------------------------------
			PSysMesh(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : Mesh(context, arena, hMyself)
			{
				m_loaded = false;
			}

			virtual ~PSysMesh() {}

			virtual void addDefaultComponents();

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_GATHER_DRAWCALLS);
			virtual void do_GATHER_DRAWCALLS(Events::Event *pEvt);

			void loadFromString_needsRC(const Particle *pSyst, const char *techName, Vector3 pSystemPos, TextureList tex, int numParticles, int &threadOwnershipMask);

			PrimitiveTypes::Float32 m_charW, m_charWAbs, m_charH, m_textLength;
			PrimitiveTypes::Bool m_loaded;
			Handle m_meshCPU;
		};

	}; // namespace Components
}; // namespace PE
#endif

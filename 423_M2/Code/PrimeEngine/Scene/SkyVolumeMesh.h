#ifndef __PYENGINE_2_0_SKY_VOLUME_MESH_H__
#define __PYENGINE_2_0_SKY_VOLUME_MESH_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes

#include "Mesh.h"


// Sibling/Children includes

namespace PE {
	namespace Components {
		struct SkyVolume : public Mesh
		{
			PE_DECLARE_CLASS(SkyVolume);

			virtual ~SkyVolume() {}

			virtual void addDefaultComponents();

			// Constructor -------------------------------------------------------------
			SkyVolume(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself, int &threadOwnershipMask);

			Handle m_meshCPU;
		};
	}; // namespace Components
}; // namespace PE
#endif

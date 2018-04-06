#ifndef __PYENGINE_2_0_PSYSSCENENODE_H__
#define __PYENGINE_2_0_PSYSSCENENODE_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "SceneNode.h"

// Particle system specific
#include "ParticleSystem.h"

//#define USE_DRAW_COMPONENT

namespace PE {
	namespace Components {
		struct PSysSceneNode : public SceneNode
		{
			PE_DECLARE_CLASS(PSysSceneNode);

			// Constructor -------------------------------------------------------------
			PSysSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

			virtual ~PSysSceneNode() {}

			void setSelfAndMeshAssetEnabled(bool enabled);

			// Component ------------------------------------------------------------

			virtual void addDefaultComponents();

			// Individual events -------------------------------------------------------

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
			virtual void do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt);

			enum DrawType
			{
				InWorld,
				InWorldFacingCamera,
				Overlay2D,
				Overlay2D_3DPos
			};
			void loadFromString_needsRC(const Particle *pSyst, DrawType drawType, Vector3 systemPos, TextureList tex, int numParticles, int &threadOwnershipMask);

			DrawType m_drawType;
			float m_scale;
			Handle m_hMyPSysMesh;
			Handle m_hMyPSysMeshInstance;
			float m_cachedAspectRatio;

			bool m_canBeRecreated;

			Matrix4x4 particle_PVWMatrix = Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			Matrix4x4 particle_scale = Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);


			// PSystem Logic and Contents -----------------------------------------------------
			//Particle particles[100];

		}; // class PSysSceneNode

	}; // namespace Components
}; // namespace PE
#endif

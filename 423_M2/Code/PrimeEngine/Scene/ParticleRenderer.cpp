// APIAbstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "../Lua/LuaEnvironment.h"

// Sibling/Children includes
#include "ParticleRenderer.h"
#include "PrimeEngine/Scene/PSysMesh.h"
#include "PrimeEngine/Scene/LineMesh.h"
#include "PrimeEngine/Scene/PSysSceneNode.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/MeshManager.h"
#include "PrimeEngine/Scene/MeshInstance.h"

const bool EnableDebugRendering = true;
namespace PE {
	namespace Components {

		using namespace PE::Events;

		PE_IMPLEMENT_CLASS1(ParticleRenderer, SceneNode);

		// Static member variables
		Handle ParticleRenderer::s_myHandle;

		// Singleton ------------------------------------------------------------------

		void ParticleRenderer::Construct(PE::GameContext &context, PE::MemoryArena arena)
		{
			Handle handle("ParticleRenderer", sizeof(ParticleRenderer));
			ParticleRenderer *pParticleRenderer = new(handle) ParticleRenderer(context, arena, handle);
			pParticleRenderer->addDefaultComponents();
			// Singleton
			SetInstanceHandle(handle);
			RootSceneNode::Instance()->addComponent(handle);
		}

		// Constructor -------------------------------------------------------------
		ParticleRenderer::ParticleRenderer(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
			: SceneNode(context, arena, hMyself)
			, m_numFreeing(0)
		{

			m_numAvaialble = NUM_PSysSceneNodes;
			for (int i = 0; i < NUM_PSysSceneNodes; i++)
				m_hAvailableSNs[i] = i;

		}

		// Methods      ------------------------------------------------------------
		void ParticleRenderer::addDefaultComponents()
		{
			Component::addDefaultComponents();

			PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, ParticleRenderer::do_PRE_GATHER_DRAWCALLS);

		}


		void ParticleRenderer::createPSysMesh(const Particle *pSyst, float timeToLive, Vector3 pos, TextureList tex, float scale, int numParticles, int &threadOwnershipMask)
		{
			if (EnableDebugRendering && m_numAvaialble)
			{


				int index = m_hAvailableSNs[--m_numAvaialble];
				Handle &h = m_hSNPool[index];
				PSysSceneNode *pPSysSN = 0;
				if (h.isValid())
				{
					//scene node has already been created
					pPSysSN = h.getObject<PSysSceneNode>();
					assert(pPSysSN->isEnabled() == false); // this SN should never be in the available list if it is enabled
					pPSysSN->setSelfAndMeshAssetEnabled(true);
				}
				else
				{
					h = PE::Handle("PARTICLE_SCENE_NODE", sizeof(PSysSceneNode));
					pPSysSN = new(h) PSysSceneNode(*m_pContext, m_arena, h);
					pPSysSN->addDefaultComponents();
					addComponent(h);
				}
				m_lifetimes[index] = timeToLive;

				PSysSceneNode::DrawType drawType = PSysSceneNode::Overlay2D_3DPos;

				Particle newPSyst[1000];

				for (int i = 0; i < numParticles; i++) {
					newPSyst[i] = pSyst[i];
					newPSyst[i].position = pPSysSN->particle_PVWMatrix * (pSyst[i].position);
					//newPSyst[i] = base->inverse() * newPSyst[i];
				}

				pPSysSN->loadFromString_needsRC(newPSyst, drawType, pos, tex, numParticles, threadOwnershipMask);
				pPSysSN->m_base.setPos(pos);
				pPSysSN->m_scale = scale;
			}
		}

		void ParticleRenderer::do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt)
		{
			// need to check lifetime here and remove whatever is out of time
			Events::Event_PRE_GATHER_DRAWCALLS *pDrawEvent = NULL;
			pDrawEvent = (Events::Event_PRE_GATHER_DRAWCALLS *)(pEvt);

			while (m_numFreeing)
			{
				m_hAvailableSNs[m_numAvaialble++] = m_hFreeingSNs[--m_numFreeing];
			}

			for (int i = 0; i < NUM_PSysSceneNodes; i++)
			{
				PE::Handle &h = m_hSNPool[i];
				if (h.isValid())
				{
					PSysSceneNode *pPSysSN = h.getObject<PSysSceneNode>();
					if (pPSysSN->isEnabled())
					{
						if (m_lifetimes[i] < 0.0f)
						{
							pPSysSN->setSelfAndMeshAssetEnabled(false);
							m_hFreeingSNs[m_numFreeing++] = i;
						}

						m_lifetimes[i] -= 1.0f;
					}
				}
			}

		}


		void ParticleRenderer::postPreDraw(int &threadOwnershipMask)
		{

		}

	}; // namespace Components
}; //namespace PE


#include "PSysSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PSysMesh.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "MeshManager.h"
#include "MeshInstance.h"

namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(PSysSceneNode, SceneNode);


		// Constructor -------------------------------------------------------------
		PSysSceneNode::PSysSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
			: SceneNode(context, arena, hMyself)
		{
			m_cachedAspectRatio = 1.0f;
			m_scale = 1.0f;
			if (IRenderer* pS = context.getGPUScreen())
				m_cachedAspectRatio = float(pS->getWidth()) / float(pS->getHeight());
		}

		void PSysSceneNode::addDefaultComponents()
		{
			SceneNode::addDefaultComponents();


			// event handlers
			PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, PSysSceneNode::do_PRE_GATHER_DRAWCALLS);
		}

		void PSysSceneNode::setSelfAndMeshAssetEnabled(bool enabled)
		{
			setEnabled(enabled);

			if (m_hMyPSysMesh.isValid())
			{
				m_hMyPSysMesh.getObject<Component>()->setEnabled(enabled);
			}
		}


		void PSysSceneNode::loadFromString_needsRC(const Particle *pSyst, DrawType drawType, Vector3 systemPos, TextureList tex, int numParticles, int &threadOwnershipMask)
		{
			m_drawType = drawType;

			PSysMesh *pPSysMesh = NULL;
			if (m_hMyPSysMesh.isValid())
			{
				pPSysMesh = m_hMyPSysMesh.getObject<PSysMesh>();
			}
			else
			{
				m_hMyPSysMesh = PE::Handle("PSYSMESH", sizeof(PSysMesh));
				pPSysMesh = new(m_hMyPSysMesh) PSysMesh(*m_pContext, m_arena, m_hMyPSysMesh);
				pPSysMesh->addDefaultComponents();

				m_pContext->getMeshManager()->registerAsset(m_hMyPSysMesh);

				m_hMyPSysMeshInstance = PE::Handle("MeshInstance", sizeof(MeshInstance));
				MeshInstance *pInstance = new(m_hMyPSysMeshInstance) MeshInstance(*m_pContext, m_arena, m_hMyPSysMeshInstance);
				pInstance->addDefaultComponents();
				pInstance->initFromRegisteredAsset(m_hMyPSysMesh);


				addComponent(m_hMyPSysMeshInstance);
			}

			PE::IRenderer::checkForErrors("");

			const char *tech = 0;
			TextureProperties currentProperties(tex);
			if (!currentProperties.isAlphaBlended)
				tech = "StdMesh_2D_Diffuse_A_RGBIntensity_Tech";
			else
				tech = "StdMesh_2D_Diffuse_A_0_Tech";




			pPSysMesh->loadFromString_needsRC(pSyst, tech, systemPos, tex, numParticles, threadOwnershipMask);
		}

		void PSysSceneNode::do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt)
		{
			Events::Event_PRE_GATHER_DRAWCALLS *pDrawEvent = NULL;
			pDrawEvent = (Events::Event_PRE_GATHER_DRAWCALLS *)(pEvt);

			Matrix4x4 projectionViewWorldMatrix = pDrawEvent->m_projectionViewTransform;
			Matrix4x4 worldMatrix;

			if (!m_hMyPSysMeshInstance.isValid())
				return;

			PSysMesh *pPSysMesh = m_hMyPSysMesh.getObject<PSysMesh>();

			if (m_drawType == InWorldFacingCamera)
			{
				m_worldTransform.turnTo(pDrawEvent->m_eyePos);
			}

			float numCharsInFullLine = 100.0f;
			numCharsInFullLine *= .5; // need to divide by 2.0 since screen goes from -1 to 1, not 0..1

			if (m_drawType == Overlay2D_3DPos)
			{
				worldMatrix = m_worldTransform;

				projectionViewWorldMatrix = projectionViewWorldMatrix * worldMatrix;

				Vector3 pos(0, 0, 0);
				pos = projectionViewWorldMatrix * pos;

				//if (pos.m_x < -1.0f || pos.m_x > 1.0f || pos.m_z <= 0.0f || pos.m_z > 1.0f)
				//{
				// this will cancel further event handling by this mesh only
				//pEvt->m_cancelSiblingAndChildEventHandling = true;
				//return;
				//}

				float factor = 0.03f;// / (numCharsInFullLine);// * m_scale;
				Matrix4x4 scale;
				scale.importScale(factor, factor*m_cachedAspectRatio, factor);

				m_worldTransform.loadIdentity();
				m_worldTransform.setPos(Vector3(pos.m_x/* - factor * pPSysMesh->m_textLength * .5f*/, pos.m_y, pos.m_z));
				m_worldTransform = m_worldTransform * scale;

				particle_scale = scale;

				// Attempt to change overall size based on proximity to camera ------------------------------------------------------------------------
				Vector3 distanceToCam = m_base.getPos() - pDrawEvent->m_eyePos;
				float distToCam = distanceToCam.length();
				float proximityFactor = 20.0f / distToCam;
				Matrix4x4 proximityScale;
				proximityScale.importScale(proximityFactor, proximityFactor / m_cachedAspectRatio, proximityFactor);
				m_worldTransform = m_worldTransform * proximityScale;
				// ------------------------------------------------------------------------------------------------------------------------------------
			}
			if (m_drawType == Overlay2D)
			{
				worldMatrix = m_worldTransform;

				float factor = 1.0f / (numCharsInFullLine)* m_scale;

				Matrix4x4 scale;
				scale.importScale(factor, factor*m_cachedAspectRatio, factor);
				m_worldTransform = worldMatrix * scale;




			}
			// PSYS ASSIGNMENT
			particle_PVWMatrix = m_worldTransform;
		}

	}; // namespace Components
}; // namespace PE

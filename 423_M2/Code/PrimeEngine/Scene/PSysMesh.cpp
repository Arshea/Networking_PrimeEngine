#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "Light.h"
#include "PrimeEngine/GameObjectModel/Camera.h"

// Sibling/Children includes
#include "PSysMesh.h"
#include "SceneNode.h"
#include "CameraManager.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(PSysMesh, Mesh);

		void PSysMesh::addDefaultComponents()
		{
			//add this handler before Mesh's handlers so we can intercept draw and modify transform
			PE_REGISTER_EVENT_HANDLER(Events::Event_GATHER_DRAWCALLS, PSysMesh::do_GATHER_DRAWCALLS);
			Mesh::addDefaultComponents();
		}

		void PSysMesh::loadFromString_needsRC(const Particle *pSyst, const char *techName, Vector3 pSystemPos, TextureList tex, int numParticles, int &threadOwnershipMask)
		{

			int len = numParticles;
			int indextorender = 0; // Render from this pos in array


			if (!m_meshCPU.isValid())
			{
				m_meshCPU = Handle("MeshCPU PSysMesh", sizeof(MeshCPU));
				new (m_meshCPU) MeshCPU(*m_pContext, m_arena);
			}
			MeshCPU &mcpu = *m_meshCPU.getObject<MeshCPU>();
			TextureProperties current_properties(tex);
			if (!m_loaded) {
				mcpu.createBillboardMeshWithColorTexture(current_properties.texture_name, "Default", 32, 32, SamplerState_NoMips_NoMinTexelLerp_NoMagTexelLerp_Clamp);
			}
			// this will cause not using the vertex buffer manager
			//so that engine always creates a new vertex buffer gpu and doesn't try to find and
			//existing one
			mcpu.m_manualBufferManagement = true;



			PositionBufferCPU *pVB = mcpu.m_hPositionBufferCPU.getObject<PositionBufferCPU>();
			TexCoordBufferCPU *pTCB = mcpu.m_hTexCoordBufferCPU.getObject<TexCoordBufferCPU>();
			NormalBufferCPU *pNB = mcpu.m_hNormalBufferCPU.getObject<NormalBufferCPU>();
			IndexBufferCPU *pIB = mcpu.m_hIndexBufferCPU.getObject<IndexBufferCPU>();
			pVB->m_values.reset(len * 4 * 3); // 4 verts * (x,y,z)
			pTCB->m_values.reset(len * 4 * 2);
			pNB->m_values.reset(len * 4 * 3);
			pIB->m_values.reset(len * 6); // 2 tris

			pIB->m_indexRanges[0].m_start = 0;
			pIB->m_indexRanges[0].m_end = len * 6 - 1;
			pIB->m_indexRanges[0].m_minVertIndex = 0;
			pIB->m_indexRanges[0].m_maxVertIndex = len * 4 - 1;

			pIB->m_minVertexIndex = pIB->m_indexRanges[0].m_minVertIndex;
			pIB->m_maxVertexIndex = pIB->m_indexRanges[0].m_maxVertIndex;

			float w = 1.0f;
			float h = 2.0f;


			m_charW = w;
			m_charH = h;
			m_charWAbs = fabs(w);
			m_textLength = (float)(len);

			// Find pSystem position for size calculations based on proximity
			CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
			Vector3 camPos = pcam->m_base.getPos(); // Camera position

			float distSystemToCam = (pSystemPos - camPos).length();

			float sizeExponent = (0.693147 /* -ln(0.5) */) / (distSystemToCam);

			float pixSize2 = 1.0f / 512.0f / 2.0f;

			for (int ic = indextorender + len - 1; ic > indextorender - 1; ic--)
			{
				float curX = pSyst[ic].position.getX();
				float curY = pSyst[ic].position.getY();

				// Adjust size based on internal particle properties
				w = 1.0f * pSyst[ic].size;
				h = w;// 2.0f * pSyst[ic].size;


					  // Adjust size based on proximity to camera (z axis)
				float localOffset = pSyst[ic].position.getZ();
				// REASONING: When localOffset == 0, sizeFactor should be 1
				// When localOffset == distSystemToCam, sizeFactor should be infinitely large
				// When localOffset == -distSystemToCam, sizeFactor should be 0.5
				// Function: y [size factor] = x [offset] ^ a + b.
				// At y = 1; x = 0; => b = 1
				// at y = 0.5; x = -distSystemToCam; => a = ((
				// This exp is calculated outside the loop above (ofc)
				w *= (exp(sizeExponent * localOffset));
				h *= (exp(sizeExponent * localOffset));
				if (w < 0 || h < 0 || localOffset >= distSystemToCam * 4.0f /* factor to cull when on screen */) { // Further than camera - do not render
					w = 0.0f;
					h = 0.0f;
				}


				/*{
				char buf[500];
				sprintf(buf, "from mesh: %d: local offset: %f, system offset: %f\n", ic, localOffset, distSystemToCam);
				OutputDebugStringA(buf);
				}*/

				// Calculate vertices positions
				Vector3 vertices[4]; // top left, top right, bottom right, bottom left
				vertices[0] = Vector3(-w / 2, h / 2, 0);
				vertices[1] = Vector3(w / 2, h / 2, 0);
				vertices[2] = Vector3(w / 2, -h / 2, 0);
				vertices[3] = Vector3(-w / 2, -h / 2, 0);
				float angle = pSyst[ic].rotation;
				for (int j = 0; j < 4; j++) {
					vertices[j] = Vector3(vertices[j].getX() * cos(angle) - vertices[j].getY() * sin(angle),
						vertices[j].getX() * sin(angle) + vertices[j].getY() * cos(angle), 0.0f); // Rotate
					vertices[j] += Vector3(curX, curY, 0.0f); // Offset
					pVB->m_values.add(vertices[j].getX(), 2 * vertices[j].getY(), vertices[j].getZ()); // Add to buffer
				} // Added factor of 2 to y -- otherwise rotation starts stretching (instead pf h = 2.0f * syst size from above) ----------------------------------------------

				pIB->m_values.add(ic * 4 + 0, ic * 4 + 1, ic * 4 + 2);
				pIB->m_values.add(ic * 4 + 2, ic * 4 + 3, ic * 4 + 0);

				//////////test///////////
				int num_cols = current_properties.num_cols;
				int num_rows = current_properties.num_rows;
				/////////////////////////
				float pixSize_horizontal = 10.0f / (current_properties.width / num_cols) / 2.0f;
				float pixSize_vertical = 10.0f / (current_properties.height / num_rows) / 2.0f;
				float dx = pixSize_horizontal;
				float dy = pixSize_vertical;
				float tcx = 1.0f / num_cols * (pSyst[ic].texCoords[0]);
				float tcy = 1.0f / num_rows * (pSyst[ic].texCoords[1]);

				pTCB->m_values.add(tcx + dx, tcy + dy); // top left
				pTCB->m_values.add(tcx + 1.0f / num_cols - dx, tcy + dy); // top right
				pTCB->m_values.add(tcx + 1.0f / num_cols - dx, tcy + 1.0f / num_rows - dy);
				pTCB->m_values.add(tcx + dx, tcy + 1.0f / num_rows - dy);

				pNB->m_values.add(0, 0, 0);
				pNB->m_values.add(0, 0, 0);
				pNB->m_values.add(0, 0, 0);
				pNB->m_values.add(0, 0, 0);
			}

			if (!m_loaded)
			{
				// first time creating gpu mesh
				loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

				if (techName)
				{
					Handle hEffect = EffectManager::Instance()->getEffectHandle(techName);

					for (unsigned int imat = 0; imat < m_effects.m_size; imat++)
					{
						if (m_effects[imat].m_size)
							m_effects[imat][0] = hEffect;
					}
				}
				m_loaded = true;
			}
			else
			{
				//just need to update vertex buffers gpu
				updateGeoFromMeshCPU_needsRC(mcpu, threadOwnershipMask);
			}
		}

		void PSysMesh::do_GATHER_DRAWCALLS(Events::Event *pEvt)
		{

		}


	}; // namespace Components
}; // namespace PE

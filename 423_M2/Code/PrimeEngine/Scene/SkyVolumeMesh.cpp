#include "SkyVolumeMesh.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "../Lua/LuaEnvironment.h"

namespace PE {
	namespace Components {
		PE_IMPLEMENT_CLASS1(SkyVolume, Mesh);

		void SkyVolume::addDefaultComponents()
		{
			Mesh::addDefaultComponents();
		}

		SkyVolume::SkyVolume(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself, int &threadOwnershipMask) : Mesh(context, arena, hMyself)
		{
			//MeshCPU m(context, arena);
			//m.ReadMesh("skyvolumemesh_mesh.mesha", "Default", "");

			if (!m_meshCPU.isValid())
			{
				m_meshCPU = Handle("MeshCPU TextMesh", sizeof(MeshCPU));
				new (m_meshCPU) MeshCPU(*m_pContext, m_arena);
			}
			MeshCPU &mcpu = *m_meshCPU.getObject<MeshCPU>();

			mcpu.createBillboardMeshWithColorTextureCubeMap("cubemap1.dds", "Default");

			// this will cause not using the vertex buffer manager
			//so that engine always creates a new vertex buffer gpu and doesn't try to find and
			//existing one
			mcpu.m_manualBufferManagement = true;

			PositionBufferCPU *pVB = mcpu.m_hPositionBufferCPU.getObject<PositionBufferCPU>();
			TexCoordBufferCPU *pTCB = mcpu.m_hTexCoordBufferCPU.getObject<TexCoordBufferCPU>();
			NormalBufferCPU *pNB = mcpu.m_hNormalBufferCPU.getObject<NormalBufferCPU>();
			IndexBufferCPU *pIB = mcpu.m_hIndexBufferCPU.getObject<IndexBufferCPU>();

			pIB->m_primitiveTopology = PEPrimitveTopology_TRIANGLES;
			pIB->m_verticesPerPolygon = 4;

			int numVerts = 36;

			pVB->m_values.reset(numVerts * 4 * 3); // 4 verts * (x,y,z)
			pTCB->m_values.reset(numVerts * 4 * 2);
			pNB->m_values.reset(numVerts * 4 * 3);
			pIB->m_values.reset(numVerts * 6); // 2 tris

			pIB->m_indexRanges[0].m_start = 0;
			pIB->m_indexRanges[0].m_end = numVerts - 1;
			pIB->m_indexRanges[0].m_minVertIndex = 0;
			pIB->m_indexRanges[0].m_maxVertIndex = numVerts - 1;

			pIB->m_minVertexIndex = pIB->m_indexRanges[0].m_minVertIndex;
			pIB->m_maxVertexIndex = pIB->m_indexRanges[0].m_maxVertIndex;

			float skyVolumeVerts[] = {
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				1.0f, -1.0f, -1.0f,
				1.0f, -1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				1.0f,  1.0f, -1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				1.0f, -1.0f,  1.0f
			};

			for (int ip = 0; ip < numVerts; ip++)
			{
				float x = skyVolumeVerts[ip * 3 + 0];
				float y = skyVolumeVerts[ip * 3 + 1];
				float z = skyVolumeVerts[ip * 3 + 2];

				pVB->m_values.add(x, y, z);
				pTCB->m_values.add(0, 0);
				pNB->m_values.add(x, y, z);
				pIB->m_values.add(ip);
			}

			loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

			//Adding technique to effects queue
			Handle hEffect = EffectManager::Instance()->getEffectHandle("Sky_Tech");

			for (unsigned int imat = 0; imat < m_effects.m_size; imat++)
			{
				if (m_effects[imat].m_size)
					m_effects[imat][0] = hEffect;
			}
		}

	}; // namespace Components
}; // namespace PE

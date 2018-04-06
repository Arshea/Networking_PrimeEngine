// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "MeshManager.h"
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

// Add debug renderer for AABB
#include "DebugRenderer.h"

namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshManager, Component);
MeshManager::MeshManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_assets(context, arena, 256)
{
}

PE::Handle MeshManager::getAsset(const char *asset, const char *package, int &threadOwnershipMask)
{
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "%s/%s", package, asset);
	
	int index = m_assets.findIndex(key);
	if (index != -1)
	{
		return m_assets.m_pairs[index].m_value;
	}
	Handle h;

	if (StringOps::endswith(asset, "skela"))
	{
		PE::Handle hSkeleton("Skeleton", sizeof(Skeleton));
		Skeleton *pSkeleton = new(hSkeleton) Skeleton(*m_pContext, m_arena, hSkeleton);
		pSkeleton->addDefaultComponents();

		pSkeleton->initFromFiles(asset, package, threadOwnershipMask);
		h = hSkeleton;
	}
	else if (StringOps::endswith(asset, "mesha"))
	{
		MeshCPU mcpu(*m_pContext, m_arena);
		mcpu.ReadMesh(asset, package, "");
		
		PE::Handle hMesh("Mesh", sizeof(Mesh));
		Mesh *pMesh = new(hMesh) Mesh(*m_pContext, m_arena, hMesh);
		pMesh->addDefaultComponents();

		pMesh->loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

		// Good place to put AABB?? 
		
		///////////////////////////////////////////////////////
		//					DOING A THING					 //
		///////////////////////////////////////////////////////

		/*PositionBufferCPU *pvbcpu = pMesh->m_hPositionBufferCPU.getObject<PositionBufferCPU>();

		for (int i = 0; i < 3; i++) {
			pMesh->min[i] = pMesh->max[i] = pvbcpu->m_values[i];
		}

		for (int i = 3; i < pvbcpu->m_values.m_size; i += 3) {
		for (int j = 0; j < 3; j++) {
		pMesh->min[j] = pMesh->min[j] < pvbcpu->m_values[i + j] ? pMesh->min[j] : pvbcpu->m_values[i + j];
		pMesh->max[j] = pMesh->max[j] > pvbcpu->m_values[i + j] ? pMesh->max[j] : pvbcpu->m_values[i + j];
		}
		}

		//PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		//SceneNode *parent = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		Handle parent = getFirstParentByType<SceneNode>();
		Matrix4x4 base;
		if (parent.isValid())
		{
			base = parent.getObject<PE::Components::SceneNode>()->m_base;


			// Add base to pMesh->min and pMesh->max points; store in imrod mesh
			pMesh->min[0] = pMesh->min[0] + base.getPos().getX();
			pMesh->min[1] = pMesh->min[1] + base.getPos().getY();
			pMesh->min[2] = pMesh->min[2] + base.getPos().getZ();
			pMesh->max[0] = pMesh->max[0] + base.getPos().getX();
			pMesh->max[1] = pMesh->max[1] + base.getPos().getY();
			pMesh->max[2] = pMesh->max[2] + base.getPos().getZ();


			// Vertices (There isn't really a clean way of doing this)
			pMesh->vertices[0] = new Vector3(pMesh->min[0], pMesh->min[1], pMesh->min[2]);
			pMesh->vertices[1] = new Vector3(pMesh->max[0], pMesh->min[1], pMesh->min[2]);
			pMesh->vertices[2] = new Vector3(pMesh->min[0], pMesh->max[1], pMesh->min[2]);
			pMesh->vertices[3] = new Vector3(pMesh->max[0], pMesh->max[1], pMesh->min[2]);
			pMesh->vertices[4] = new Vector3(pMesh->min[0], pMesh->min[1], pMesh->max[2]);
			pMesh->vertices[5] = new Vector3(pMesh->max[0], pMesh->min[1], pMesh->max[2]);
			pMesh->vertices[6] = new Vector3(pMesh->min[0], pMesh->max[1], pMesh->max[2]);
			pMesh->vertices[7] = new Vector3(pMesh->max[0], pMesh->max[1], pMesh->max[2]);



			Vector3 color = { 1.0f, 1.0f, 0.0f };
			Vector3 linepts[48];

			for (int i = 0; i < 24; i++) {
				linepts[2 * i + 1] = color;
			}

			// Array list of vertices to include.
			int verticesToAdd[] = { 0,1,0,2,0,4,3,1,3,2,3,7,5,1,5,4,5,7,6,2,6,4,6,7 };
			for (int i = 0; i < 24; i++) {
				linepts[2 * i] = *pMesh->vertices[verticesToAdd[i]];
				linepts[2 * i + 1] = color;
			}

			DebugRenderer::Instance()->createLineMesh(true, base, &linepts[0].m_x, 24, 1000000);
			OutputDebugStringA("In if clause!!!!!! MeshManager \n");
		}
		else {
			OutputDebugStringA("In else clause!!!!!! MeshManager \n");
		}*/
		

#if PE_API_IS_D3D11
		// todo: work out how lods will work
		//scpu.buildLod();
#endif
		  // generate collision volume here. or you could generate it in MeshCPU::ReadMesh()
		  pMesh->m_performBoundingVolumeCulling = true; // will now perform tests for this mesh

		h = hMesh;
	}


	PEASSERT(h.isValid(), "Something must need to be loaded here");

	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
	return h;
}

void MeshManager::registerAsset(const PE::Handle &h)
{
	static int uniqueId = 0;
	++uniqueId;
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "__generated_%d", uniqueId);
	
	int index = m_assets.findIndex(key);
	PEASSERT(index == -1, "Generated meshes have to be unique");
	
	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
}

}; // namespace Components
}; // namespace PE

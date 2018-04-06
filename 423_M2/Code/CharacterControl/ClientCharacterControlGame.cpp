
#include "ClientCharacterControlGame.h"

#include "PrimeEngine/Scene/SkeletonInstance.h"

#include "ClientGameObjectManagerAddon.h"
#include "Characters/Flyer.h"
#include "Tank/ClientTank.h"
#include "Client/ClientSpaceShipControls.h"
#include "Characters/SoldierNPCAnimationSM.h"
#include "CharacterControl/Characters/SoldierNPCAnimationSM.h"
#include "CharacterControlContext.h"
#include "PrimeEngine/Scene/SkyVolumeMesh.h"
#if PE_PLAT_IS_WIN32
#include "test.h"
#endif

using namespace PE;
using namespace PE::Components;

namespace CharacterControl {
namespace Components {

// is run after initializing the engine
// basic demo just adds a light source and a default control scheme
// so that uses can add simple objects like meshes, skins, light sources, levels, etc. from asset manager
int ClientCharacterControlGame::initGame()
{
	// super implementation
	ClientGame::initGame();

	//add game specific context
	CharacterControlContext *pGameCtx = new (m_arena) CharacterControlContext;

	m_pContext->m_pGameSpecificContext = pGameCtx;

	PE::Components::LuaEnvironment *pLuaEnv = m_pContext->getLuaEnvironment();
	
	// init events, components, and other classes of the project
	CharacterControl::Register(pLuaEnv, PE::GlobalRegistry::Instance());

	// grey-ish background
	m_pContext->getGPUScreen()->setClearColor(Vector4(0, 0, 0, 0.0f));


	// game controls read input queue and post events onto general queue
	// the events from general queue are then passed on to game components
	PE::Handle hDefaultGameControls("GAME_CONTROLS", sizeof(DefaultGameControls));
	m_pContext->m_pDefaultGameControls = new(hDefaultGameControls) DefaultGameControls(*m_pContext, m_arena, hDefaultGameControls);
	m_pContext->m_pDefaultGameControls->addDefaultComponents();

	m_pContext->getGameObjectManager()->addComponent(hDefaultGameControls);

	// initialize game

	// create the GameObjectmanager addon that is in charge of game objects in this demo
	{
		// create the GameObjectmanager addon that is in charge of game objects in this demo
        PE::Handle hGOMAddon = PE::Handle("ClientGameObjectManagerAddon", sizeof(ClientGameObjectManagerAddon));
		pGameCtx->m_pGameObjectManagerAddon = new(hGOMAddon) ClientGameObjectManagerAddon(*m_pContext, m_arena, hGOMAddon);
		pGameCtx->getGameObjectManagerAddon()->addDefaultComponents();
	
		// add it to game object manager
		// now all game events will be passed through to our GameObjectManagerAddon
		m_pContext->getGameObjectManager()->addComponent(hGOMAddon);
	}

	bool spawnALotOfSoldiersForGpuAnim = false;

	//create tank controls that will be enabled if tank is activated
	{
		// create the GameObjectmanager addon that is in charge of game objects in this demo
		PE::Handle h("TankGameControls", sizeof(TankGameControls));
		pGameCtx->m_pTankGameControls = new(h) TankGameControls(*m_pContext, m_arena, h);
		pGameCtx->getTankGameControls()->addDefaultComponents();

		// add it to game object manager addon
		pGameCtx->getGameObjectManagerAddon()->addComponent(h);

		// start deactivated. needs to be deactivated AFTER adding it to parent components
		pGameCtx->getTankGameControls()->setEnabled(false);

		
#if !PE_API_IS_D3D11
		if (!spawnALotOfSoldiersForGpuAnim)
		{
			//for (int i = 0; i < 6; ++i)
				((ClientGameObjectManagerAddon*)(pGameCtx->getGameObjectManagerAddon()))->createTank(
				-4, m_pContext->m_gameThreadThreadOwnershipMask);
				((ClientGameObjectManagerAddon*)(pGameCtx->getGameObjectManagerAddon()))->createTank(
					4, m_pContext->m_gameThreadThreadOwnershipMask);
		}
#endif
	}

	{
		PE::Handle h("ClientSpaceShipControls", sizeof(SpaceShipGameControls));
		pGameCtx->m_pSpaceShipGameControls = new(h) SpaceShipGameControls(*m_pContext, m_arena, h);
		pGameCtx->getSpaceShipGameControls()->addDefaultComponents();

		// add it to game object manager addon
		pGameCtx->getGameObjectManagerAddon()->addComponent(h);

		// start deactivated. needs to be deactivated AFTER adding it to parent components
		pGameCtx->getSpaceShipGameControls()->setEnabled(false);

		if (false)
		{
			((ClientGameObjectManagerAddon*)(pGameCtx->getGameObjectManagerAddon()))->createSpaceShip(
				m_pContext->m_gameThreadThreadOwnershipMask);
		}
	}

	// manually add flyer
	if (false)
	{
		PE::Handle hF("Flyer", sizeof(Flyer));
		Flyer *pF = new(hF) Flyer(*m_pContext, m_arena, hF, m_pContext->m_gameThreadThreadOwnershipMask);
		pF->addDefaultComponents();
		pGameCtx->getGameObjectManagerAddon()->addComponent(hF);
	}

	if (false)
	{
		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		pSN->m_base.setPos(Vector3(0, 0, 0));

		{
			PE::Handle hSoldierAnimSM("SoldierNPCAnimationSM", sizeof(SoldierNPCAnimationSM));
			SoldierNPCAnimationSM *pSoldierAnimSM = new(hSoldierAnimSM) SoldierNPCAnimationSM(*m_pContext, m_arena, hSoldierAnimSM);
			pSoldierAnimSM->addDefaultComponents();

			pSoldierAnimSM->m_debugAnimIdOffset = 0;// rand() % 3;


			PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
			SkeletonInstance *pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance, 
				hSoldierAnimSM);
			pSkelInst->addDefaultComponents();

			pSkelInst->initFromFiles("soldier_Soldier_Skeleton.skela", "Default", m_pContext->m_gameThreadThreadOwnershipMask);
			pSkelInst->setAnimSet("soldier_Soldier_Skeleton.animseta", "Default");

					
			{
				PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
				MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
				pMeshInstance->addDefaultComponents();
				
				pMeshInstance->initFromFile("SoldierTransform.mesha", "Default", m_pContext->m_gameThreadThreadOwnershipMask);
				
				pSkelInst->addComponent(hMeshInstance);
			}

			
// 			{
// 				// create a scene node for gun attached to a joint
// 				PE::Handle hMyGunSN = PE::Handle("SCENE_NODE", sizeof(JointSceneNode));
// 				JointSceneNode *pGunSN = new(hMyGunSN) JointSceneNode(*m_pContext, m_arena, hMyGunSN, 38);
// 				pGunSN->addDefaultComponents();
// 				{
// 					PE::Handle hMyGunMesh = PE::Handle("MeshInstance", sizeof(MeshInstance));
// 					MeshInstance *pGunMeshInstance = new(hMyGunMesh) MeshInstance(*m_pContext, m_arena, hMyGunMesh);
// 
// 					pGunMeshInstance->addDefaultComponents();
// 					pGunMeshInstance->initFromFile(pEvt->m_gunMeshName, pEvt->m_gunMeshPackage, pEvt->m_threadOwnershipMask);
// 
// 					// add gun to joint
// 					pGunSN->addComponent(hMyGunMesh);
// 				}
// 				// add gun scene node to the skin
// 				pSkelInst->addComponent(hMyGunSN);
// 			}

		
			Events::SoldierNPCAnimSM_Event_WALK evt;
			pSkelInst->handleEvent(&evt);

			// add skeleton to scene node
			pSN->addComponent(hSkeletonInstance);
		}
		
		RootSceneNode::Instance()->addComponent(hSN);
	}

	//the soldier creation code expects not having the redner context so release it here, and reacquire afterwards
	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	
	#if PE_API_IS_D3D11
	if (spawnALotOfSoldiersForGpuAnim)
	{
		int smallx = 4;

		for (int y = 0; y < 16; ++y)
			for (int x = 0; x < smallx; ++x)
				((ClientGameObjectManagerAddon*)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon()))->createSoldierNPC(
				Vector3(x * 2.0f , 0.0f, 2.0f * y), m_pContext->m_gameThreadThreadOwnershipMask);
	}
	#endif
    
    m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	
    bool spawnALotOfMeshes = false;
    
    int maxX = 10; // maybe need more to get framerate lower
    
    if (spawnALotOfMeshes)
    {
        for (int ix = 0; ix < maxX; ++ix)
        {
            for (int iy = 0; iy < 10; ++iy)
            {
                PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
                SceneNode *pMainSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
                pMainSN->addDefaultComponents();
        
                pMainSN->m_base.setPos(Vector3(ix * 2.0f, 0, -10.0f -iy * 2.0f));
        
                PE::Handle hImrodMeshInst = PE::Handle("MeshInstance", sizeof(MeshInstance));
                MeshInstance *pImrodMeshInst = new(hImrodMeshInst) MeshInstance(*m_pContext, m_arena, hImrodMeshInst);
        
                pImrodMeshInst->addDefaultComponents();
				pImrodMeshInst->initFromFile("imrod.x_imrodmesh_mesh.mesha", "Default", m_pContext->    m_gameThreadThreadOwnershipMask);

                pMainSN->addComponent(hImrodMeshInst);

				// CSCI 522 Assignment 3 (culling with AABB and camera frustum) edits start here



				// Moved from MeshManager.cpp 
				PositionBufferCPU *pvbcpu = pImrodMeshInst->m_hAsset.getObject<Mesh>()->m_hPositionBufferCPU.getObject<PositionBufferCPU>();

				// Min and Max coordinates for AABB
				PrimitiveTypes::Float32 min[3];
				PrimitiveTypes::Float32 max[3];

				// Initialise
				for (int i = 0; i < 3; i++) {
					min[i] = max[i] = pvbcpu->m_values[i];
				}

				// Get mins and maxes
				for (int i = 3; i < pvbcpu->m_values.m_size; i += 3) {
					for (int j = 0; j < 3; j++) {
						min[j] = min[j] < pvbcpu->m_values[i + j] ? min[j] : pvbcpu->m_values[i + j];
						max[j] = max[j] > pvbcpu->m_values[i + j] ? max[j] : pvbcpu->m_values[i + j];
					}
				} 


				Matrix4x4 base = pMainSN->m_base;


				// Add base to min and max points; store in imrod mesh
				pImrodMeshInst->min[0] = min[0] + base.getPos().getX();
				pImrodMeshInst->min[1] = min[1] + base.getPos().getY();
				pImrodMeshInst->min[2] = min[2] + base.getPos().getZ();
				pImrodMeshInst->max[0] = max[0] + base.getPos().getX();
				pImrodMeshInst->max[1] = max[1] + base.getPos().getY();
				pImrodMeshInst->max[2] = max[2] + base.getPos().getZ();


				// Vertices (There isn't really a clean way of doing this)
				pImrodMeshInst->vertices[0] = new Vector3(pImrodMeshInst->min[0], pImrodMeshInst->min[1], pImrodMeshInst->min[2]);
				pImrodMeshInst->vertices[1] = new Vector3(pImrodMeshInst->max[0], pImrodMeshInst->min[1], pImrodMeshInst->min[2]);
				pImrodMeshInst->vertices[2] = new Vector3(pImrodMeshInst->min[0], pImrodMeshInst->max[1], pImrodMeshInst->min[2]);
				pImrodMeshInst->vertices[3] = new Vector3(pImrodMeshInst->max[0], pImrodMeshInst->max[1], pImrodMeshInst->min[2]);
				pImrodMeshInst->vertices[4] = new Vector3(pImrodMeshInst->min[0], pImrodMeshInst->min[1], pImrodMeshInst->max[2]);
				pImrodMeshInst->vertices[5] = new Vector3(pImrodMeshInst->max[0], pImrodMeshInst->min[1], pImrodMeshInst->max[2]);
				pImrodMeshInst->vertices[6] = new Vector3(pImrodMeshInst->min[0], pImrodMeshInst->max[1], pImrodMeshInst->max[2]);
				pImrodMeshInst->vertices[7] = new Vector3(pImrodMeshInst->max[0], pImrodMeshInst->max[1], pImrodMeshInst->max[2]);


				
				Vector3 color = { 1.0f, 1.0f, 0.0f };
				Vector3 linepts[48];

				/*for (int i = 0; i < 24; i++) {
					linepts[2 * i + 1] = color;
				}*/

				// Array list of vertices to include.
				int verticesToAdd[] = { 0,1,0,2,0,4,3,1,3,2,3,7,5,1,5,4,5,7,6,2,6,4,6,7 };
				for (int i = 0; i < 24; i++) {
					linepts[2 * i] = *pImrodMeshInst->vertices[verticesToAdd[i]];
					linepts[2 * i + 1] = color;
				}
				
				DebugRenderer::Instance()->createLineMesh(true, base, &linepts[0].m_x, 24, 100000);

                RootSceneNode::Instance()->addComponent(hSN);
            }
        }
    }

#	if APIABSTRACTION_D3D11
	//Skybox stuff
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pMainSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pMainSN->addDefaultComponents();

	PE::Handle hSkyboxInst = PE::Handle("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pSkyboxInst = new(hSkyboxInst) MeshInstance(*m_pContext, m_arena, hSkyboxInst);
	pSkyboxInst->addDefaultComponents();

	PE::Handle hSkybox = PE::Handle("SkyVolume", sizeof(SkyVolume));
	SkyVolume *pSkybox = new(hSkybox) SkyVolume(*m_pContext, m_arena, hSkybox, m_pContext->m_gameThreadThreadOwnershipMask);
	pSkybox->addDefaultComponents();

	m_pContext->getMeshManager()->registerAsset(hSkybox);
	pSkyboxInst->initFromRegisteredAsset(hSkybox);
	pMainSN->addComponent(hSkyboxInst);

	RootSceneNode::Instance()->addComponent(hSN);
#endif
/*
#if PE_PLAT_IS_WIN32
	
	int testVar = number_cpp_extern + number_c_extern;// + TestExternIntVar;

	__asm {
		mov eax, [number_cpp_extern]
		add eax, [number_c_extern]
		mov [testVar], eax
		mov eax, [number_c_extern]
	}

	testfunc_c_cdecl();

	testfunc_c_stdcall();

	testfunc_c_fastcall();
#endif
	*/
	//here's how you would run this level trough code

	//whenever Lua Code is executed it assumes that the thread DOES NOT HAVE render context
	//so in this case we need to release render context (this function has render context)
	//and then reacquire once lua is done

	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

#if PE_PLAT_IS_PSVITA // do it for ps3 becasue right now communication between pyClient and ps3 is not working
	//m_pContext->getLuaEnvironment()->runString("LevelLoader.loadLevel('assignment4_demo.x_level.levela', 'CharacterControl')");
#endif
	//m_pContext->getLuaEnvironment()->runString("LevelLoader.loadLevel('assignment4_demo.x_level.levela', 'CharacterControl')");
	//m_pContext->getLuaEnvironment()->runString("LevelLoader.loadLevel('char_highlight.x_level.levela', 'Basic')");
	//m_pContext->getLuaEnvironment()->runString("LevelLoader.loadLevel('floorIsLava.x_level.levela', 'CharacterControl')");
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

	return 1; // 1 (true) = success. no errors. TODO: add error checking
}


}
}

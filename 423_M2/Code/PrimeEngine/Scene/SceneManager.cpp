// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "SceneManager.h"
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
#include "PrimeEngine/Scene/DebugRenderer.h"
namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(SceneManager, Component);
		SceneManager::SceneManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
			: Component(context, arena, hMyself)
		{
			current_scene = START;
			for (int i = 0; i < END + 1; i++) {
				numLoads[i] = 0;
				remain[i] = true;
			}
				
		}

		void SceneManager::addDefaultComponents()
		{
			Component::addDefaultComponents();

		}
		void SceneManager::setCurrentScene(SCENE_TYPES scene_id) {
			current_scene = scene_id;
			if (current_scene > START) {
				remain[current_scene - 1] = false;
			}
		}

		char* SceneManager::getCurrentScene() {
			switch (current_scene) {
			case START:
				return "LevelLoader.loadLevel('start_screen.x_level.levela', 'CharacterControl')";
				break;
			case LEVEL_1:
				return "LevelLoader.loadLevel('floorIsLava.x_level.levela', 'CharacterControl')";
				break;
			case END:
				return "LevelLoader.loadLevel('start_screen.x_level.levela', 'CharacterControl')";
				break;
			}
		}
		
		int SceneManager::numberOfSceneLoads() {
			return numLoads[current_scene];
		}

		void SceneManager::loadCurrentScene() {
			numLoads[current_scene]++;
		}

		bool SceneManager::remainInCurrentScene() {
			return remain[current_scene];
		}

		void SceneManager::printInCurrentScene(int &threadownershipmask) {
			switch (current_scene) {
			case START:
				DebugRenderer::Instance()->createTextMesh(
					"FLOOR IS LAVA", true, false, false, false, 1,
					Vector3(0.05f, .5f, 0), 7.0f, threadownershipmask);
				DebugRenderer::Instance()->createTextMesh(
					"Press P to play", true, false, false, false, 1,
					Vector3(0.35f, .8f, 0), 2.0f, threadownershipmask);
				return;
			}
			
		}
	};
};
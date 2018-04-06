#ifndef __PYENGINE_2_0_SCENEMANAGER_H__
#define __PYENGINE_2_0_SCENEMANAGER_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "../Geometry/MeshCPU/MeshCPU.h"
#include "../Math/Matrix4x4.h"

#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPU.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/IndexBufferGPU.h"

#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

// Sibling/Children includes

namespace PE {
	namespace Components {
		enum SCENE_TYPES {
			START,
			LEVEL_1,
			END
		};
		struct SceneManager : Component
		{
			PE_DECLARE_CLASS(SceneManager);
			SceneManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);
			int numLoads[END+1];
			bool remain[END + 1];

			void setCurrentScene(SCENE_TYPES scene_id);
			char* getCurrentScene();
			int numberOfSceneLoads();
			void loadCurrentScene();
			bool remainInCurrentScene();
			void printInCurrentScene(int &threadownershipmask);
			virtual void addDefaultComponents();
			SCENE_TYPES current_scene;
		};

	}; // namespace Components
}; // namespace PE
#endif
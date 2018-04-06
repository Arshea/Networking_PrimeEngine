#ifndef _TANK_H_
#define _TANK_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Vector3.h"

// Martin Testing
#include "PrimeEngine/Scene/DebugRenderer.h"

#include "PrimeEngine/Physics/PhysicsComponent.h"

#define SERVER_MOVEMENT false // Change this to true to handle movement / physics on server; false for client


namespace PE {
	namespace Events {
		struct EventQueueManager;
	}
}

namespace CharacterControl {
	namespace Components {

		struct TankGameControls : public PE::Components::Component
		{
			PE_DECLARE_CLASS(TankGameControls);

		public:

			TankGameControls(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
				: PE::Components::Component(context, arena, hMyself)
			{
				is_reset = false;
				has_won = false;
			}

			virtual ~TankGameControls() {}
			// Component ------------------------------------------------------------

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
			virtual void do_UPDATE(PE::Events::Event *pEvt);

			virtual void addDefaultComponents();

			//Methods----------------
			void handleIOSDebugInputEvents(PE::Events::Event *pEvt);
			Vector3 handleKeyboardDebugInputEvents(PE::Events::Event *pEvt);
			void handleControllerDebugInputEvents(PE::Events::Event *pEvt);

			PE::Events::EventQueueManager *m_pQueueManager;

			PrimitiveTypes::Float32 m_frameTime;

			//If true, player respawns at start point
			bool is_reset;
			bool has_won;
		};

		struct TankController : public PE::Components::Component
		{
			// component API
			PE_DECLARE_CLASS(TankController);

			TankController(PE::GameContext &context, PE::MemoryArena arena,
				PE::Handle myHandle, float speed,
				Vector3 spawnPos, float networkPingInterval); // constructor

			virtual void addDefaultComponents(); // adds default children and event handlers


			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
			virtual void do_UPDATE(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Throttle);
			virtual void do_Tank_Throttle(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Turn);
			virtual void do_Tank_Turn(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Jump);
			virtual void do_Tank_Jump(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Camera_Vert_Orbit);
			virtual void do_Tank_Camera_Vert_Orbit(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Respawn);
			virtual void do_Respawn(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Stop);
			virtual void do_Stop(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Tank_Throttle_Server);
			virtual void do_Tank_Throttle_Server(PE::Events::Event *pEvt);

			void overrideTransform(Matrix4x4 &t);
			void activate();


			float m_timeSpeed;
			float m_time;
			float m_networkPingTimer;
			float m_networkPingInterval;
			Vector2 m_center;
			PrimitiveTypes::UInt32 m_counter;
			Vector3 m_spawnPos;
			bool m_active;
			bool m_overriden;
			Matrix4x4 m_transformOverride;


			PhysicsComponent my_phys;
			float currentCamOrbitAngle = 0.5f; // Between -PI/2 and PI/2 for camera elevation (vertical orbit position)
											   // -PI/2 means looking up from below; -PI/2 means looking down from above
			float cameraToPlayerDist = 8.0f; // Distance from camera to player
		};
	}; // namespace Components
}; // namespace CharacterControl

#endif

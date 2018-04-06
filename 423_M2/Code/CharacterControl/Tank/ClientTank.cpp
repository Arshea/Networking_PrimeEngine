#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Inter-Engine includes
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/Mesh.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Networking/EventManager.h"
#include "PrimeEngine/Networking/Client/ClientNetworkManager.h"
#include "CharacterControl/Events/Events.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"
#include "PrimeEngine/Events/StandardKeyboardEvents.h"
#include "PrimeEngine/Events/StandardIOSEvents.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"
#include "PrimeEngine/Events/StandardControllerEvents.h"
#include "PrimeEngine/GameObjectModel/DefaultGameControls/DefaultGameControls.h"
#include "CharacterControl/CharacterControlContext.h"
#include "CharacterControl/Characters/SoldierNPCAnimationSM.h"
#include "CharacterControl/Characters/SoldierNPC.h"
#include "ClientTank.h"
#include "CharacterControl/Client/ClientSpaceShipControls.h"
#include "PrimeEngine/Scene/SceneManager.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

// Arkane Control Values
#define Analog_To_Digital_Trigger_Distance 0.5f
static float Debug_Fly_Speed = 8.0f; //Units per second
#define Debug_Rotate_Speed 2.0f //Radians per second
#define Player_Keyboard_Rotate_Speed 20.0f //Radians per second
Vector3 relative_movement_global;
PrimitiveTypes::Float32 frame_time_global;
bool is_playing = false;
namespace CharacterControl {
	namespace Components {


		PE_IMPLEMENT_CLASS1(TankGameControls, PE::Components::Component);

		void TankGameControls::addDefaultComponents()
		{
			Component::addDefaultComponents();

			PE_REGISTER_EVENT_HANDLER(Event_UPDATE, TankGameControls::do_UPDATE);
		}

		void TankGameControls::do_UPDATE(PE::Events::Event *pEvt)
		{

			Vector3 intendedMovement = Vector3(0.0f, 0.0f, 0.0f);

			// Process input events (XBOX360 buttons, triggers...)
			PE::Handle iqh = PE::Events::EventQueueManager::Instance()->getEventQueueHandle("input");

			// Process input event -> game event conversion
			while (!iqh.getObject<PE::Events::EventQueue>()->empty())
			{
				PE::Events::Event *pInputEvt = iqh.getObject<PE::Events::EventQueue>()->getFront();
				m_frameTime = ((Event_UPDATE*)(pEvt))->m_frameTime;
				
				// Have DefaultGameControls translate the input event to GameEvents
				intendedMovement += handleKeyboardDebugInputEvents(pInputEvt);

				//handleControllerDebugInputEvents(pInputEvt);
				//handleIOSDebugInputEvents(pInputEvt);

				iqh.getObject<PE::Events::EventQueue>()->destroyFront();
			}

			//// Events are destoryed by destroyFront() but this is called every frame just in case
			//iqh.getObject<PE::Events::EventQueue>()->destroy();

			if (!is_reset) {
				// Actually handle the (accumulated) movement
				if (!SERVER_MOVEMENT) {
					PE::Handle h("EVENT", sizeof(Events::Event_Tank_Throttle));
					Events::Event_Tank_Throttle *throttleEvt = new(h) Events::Event_Tank_Throttle;

					throttleEvt->m_relativeMove = intendedMovement;
					throttleEvt->m_frameTime = m_frameTime;
					if (m_pQueueManager != nullptr) m_pQueueManager->add(h, QT_GENERAL); // Null until first keypress
					else h.release();
				}

				//input was stopped
				if (intendedMovement == Vector3(0.0f, 0.0f, 0.0f)) 
				{
					PE::Handle h("EVENT", sizeof(Events::Event_Tank_STOP));
					Events::Event_Tank_STOP *flyCameraEvt = new(h) Events::Event_Tank_STOP;
					if (m_pQueueManager != nullptr) m_pQueueManager->add(h, QT_GENERAL);
					//h.release();
				
				}
			}
			else {
				PE::Handle h("EVENT", sizeof(Event_Respawn));
				Event_Respawn *respawn_event = new(h) Event_Respawn;

				m_pQueueManager->add(h, QT_GENERAL);

			}


			frame_time_global = m_frameTime;

			if (SERVER_MOVEMENT) {
				relative_movement_global = intendedMovement;
			}
		}

		void TankGameControls::handleIOSDebugInputEvents(Event *pEvt)
		{
#if APIABSTRACTION_IOS
			m_pQueueManager = PE::Events::EventQueueManager::Instance();
			if (Event_IOS_TOUCH_MOVED::GetClassId() == pEvt->getClassId())
			{
				Event_IOS_TOUCH_MOVED *pRealEvent = (Event_IOS_TOUCH_MOVED *)(pEvt);

				if (pRealEvent->touchesCount > 1)
				{
					PE::Handle h("EVENT", sizeof(Events::Event_Tank_Throttle));
					Events::Event_Tank_Throttle *flyCameraEvt = new(h) Events::Event_Tank_Throttle;

					Vector3 relativeMovement(0.0f, 0.0f, -30.0f * pRealEvent->m_normalized_dy);
					flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}
				else
				{
					PE::Handle h("EVENT", sizeof(Event_Tank_Turn));
					Event_Tank_Turn *rotateCameraEvt = new(h) Event_Tank_Turn;

					Vector3 relativeRotate(pRealEvent->m_normalized_dx * 10, 0.0f, 0.0f);
					rotateCameraEvt->m_relativeRotate = relativeRotate * Debug_Rotate_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}
			}
#endif
		}

		Vector3 TankGameControls::handleKeyboardDebugInputEvents(Event *pEvt)
		{
			
			if (!is_playing) {
				if (PE::Events::Event_KEY_P_HELD::GetClassId() == pEvt->getClassId())
				{
					SceneManager *scenemanager = m_pContext->getSceneManager();
					scenemanager->setCurrentScene(LEVEL_1);
					is_playing = true;
				}
			}
			m_pQueueManager = PE::Events::EventQueueManager::Instance();
			if(is_playing){
				if (PE::Events::Event_KEY_A_HELD::GetClassId() == pEvt->getClassId())
				{
					is_reset = false;
					return Vector3(-1.0f, 0.0f, 0.0f);

					//PE::Handle h("EVENT", sizeof(Events::Event_Tank_Throttle));
					//Events::Event_Tank_Throttle *flyCameraEvt = new(h) Events::Event_Tank_Throttle;
					//
					//Vector3 relativeMovement(-1.0f,0.0f,0.0f);
					//flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
					//m_pQueueManager->add(h, QT_GENERAL);
				}
				else

					if (Event_KEY_S_HELD::GetClassId() == pEvt->getClassId())
					{
						is_reset = false;
						return Vector3(0.0f, 0.0f, -1.0f);

					}

					else if (Event_KEY_D_HELD::GetClassId() == pEvt->getClassId())
					{
						is_reset = false;
						return Vector3(1.0f, 0.0f, 0.0f);

					}
					else if (Event_KEY_W_HELD::GetClassId() == pEvt->getClassId())
					{
						is_reset = false;
						return Vector3(0.0f, 0.0f, 1.0f);

					}
					else if (Event_KEY_LEFT_HELD::GetClassId() == pEvt->getClassId())
					{
						is_reset = false;
						PE::Handle h("EVENT", sizeof(Event_Tank_Turn));
						Event_Tank_Turn *rotateCameraEvt = new(h) Event_Tank_Turn;

						Vector3 relativeRotate(-1.0f, 0.0f, 0.0f);
						rotateCameraEvt->m_relativeRotate = relativeRotate * Debug_Rotate_Speed * m_frameTime;
						m_pQueueManager->add(h, QT_GENERAL);
					}
					else if (Event_KEY_RIGHT_HELD::GetClassId() == pEvt->getClassId())
					{
						PE::Handle h("EVENT", sizeof(Event_Tank_Turn));
						Event_Tank_Turn *rotateCameraEvt = new(h) Event_Tank_Turn;

						Vector3 relativeRotate(1.0f, 0.0f, 0.0f);
						rotateCameraEvt->m_relativeRotate = relativeRotate * Debug_Rotate_Speed * m_frameTime;
						m_pQueueManager->add(h, QT_GENERAL);
					}

					else if (Event_KEY_DOWN_HELD::GetClassId() == pEvt->getClassId())
					{
						PE::Handle h("EVENT", sizeof(Event_Tank_Camera_Vert_Orbit));
						Event_Tank_Camera_Vert_Orbit *rotateCameraEvt = new(h) Event_Tank_Camera_Vert_Orbit;

						rotateCameraEvt->m_relativeRotate = -Debug_Rotate_Speed * m_frameTime; // Add some multiplier for speed?
						m_pQueueManager->add(h, QT_GENERAL);
					}
					else if (Event_KEY_UP_HELD::GetClassId() == pEvt->getClassId())
					{
						PE::Handle h("EVENT", sizeof(Event_Tank_Camera_Vert_Orbit));
						Event_Tank_Camera_Vert_Orbit *rotateCameraEvt = new(h) Event_Tank_Camera_Vert_Orbit;

						rotateCameraEvt->m_relativeRotate = Debug_Rotate_Speed * m_frameTime; // Add some multiplier for speed?
						m_pQueueManager->add(h, QT_GENERAL);
					}

				// Also need to only implement if grounded...........
					else if (Event_KEY_SPACE_HELD::GetClassId() == pEvt->getClassId())
					{
						// jump.
						// Could make a separate event?
						//-shabs-
						PE::Handle h("EVENT", sizeof(Events::Event_Tank_Jump));
						Events::Event_Tank_Jump *flyCameraEvt = new(h) Events::Event_Tank_Jump;

						//flyCameraEvt->accMovement = intendedMovement;
						//send Jump event
						m_pQueueManager->add(h, QT_GENERAL);
						return Vector3(0.0f, 1.0f, 0.0f);
						// The 1.0 isn't the velocity. Just a flag to tell it you're trying to jump.
					}
					else if (Event_KEY_L_HELD::GetClassId() == pEvt->getClassId())
					{
						is_reset = true;
					}
					else if (Event_KEY_K_HELD::GetClassId() == pEvt->getClassId()) {
						//if (!has_won) {
						//	CharacterControl::Events::Event_Server_Win evt(*m_pContext);
						//	//evt.m_transform = pFirstSN->m_base;

						//	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
						//	pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);
						//	has_won = true;
						//}
					}
					else
					{
						Component::handleEvent(pEvt);
					}
			}
			return Vector3(0.0f, 0.0f, 0.0f);
		}

		void TankGameControls::handleControllerDebugInputEvents(Event *pEvt)
		{

			if (Event_ANALOG_L_THUMB_MOVE::GetClassId() == pEvt->getClassId())
			{
				Event_ANALOG_L_THUMB_MOVE *pRealEvent = (Event_ANALOG_L_THUMB_MOVE*)(pEvt);

				//throttle
				{
					PE::Handle h("EVENT", sizeof(Events::Event_Tank_Throttle));
					Events::Event_Tank_Throttle *flyCameraEvt = new(h) Events::Event_Tank_Throttle;

					Vector3 relativeMovement(0.0f, 0.0f, pRealEvent->m_absPosition.getY());
					flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}

				//turn
				{
					PE::Handle h("EVENT", sizeof(Event_Tank_Turn));
					Event_Tank_Turn *rotateCameraEvt = new(h) Event_Tank_Turn;

					Vector3 relativeRotate(pRealEvent->m_absPosition.getX(), 0.0f, 0.0f);
					rotateCameraEvt->m_relativeRotate = relativeRotate * Debug_Rotate_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}
			}
			/*
			else if (Event_ANALOG_R_THUMB_MOVE::GetClassId() == pEvt->getClassId())
			{
			Event_ANALOG_R_THUMB_MOVE *pRealEvent = (Event_ANALOG_R_THUMB_MOVE *)(pEvt);

			PE::Handle h("EVENT", sizeof(Event_ROTATE_CAMERA));
			Event_ROTATE_CAMERA *rotateCameraEvt = new(h) Event_ROTATE_CAMERA ;

			Vector3 relativeRotate(pRealEvent->m_absPosition.getX(), pRealEvent->m_absPosition.getY(), 0.0f);
			rotateCameraEvt->m_relativeRotate = relativeRotate * Debug_Rotate_Speed * m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);
			}
			else if (Event_PAD_N_DOWN::GetClassId() == pEvt->getClassId())
			{
			}
			else if (Event_PAD_N_HELD::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_N_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_S_DOWN::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_S_HELD::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_S_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_W_DOWN::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_W_HELD::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_W_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_E_DOWN::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_E_HELD::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_PAD_E_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_A_HELD::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_Y_DOWN::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_A_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_B_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_X_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_Y_UP::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_ANALOG_L_TRIGGER_MOVE::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_ANALOG_R_TRIGGER_MOVE::GetClassId() == pEvt->getClassId())
			{

			}
			else if (Event_BUTTON_L_SHOULDER_DOWN::GetClassId() == pEvt->getClassId())
			{

			}
			else
			*/
			{
				Component::handleEvent(pEvt);
			}
		}

		PE_IMPLEMENT_CLASS1(TankController, Component);

		TankController::TankController(PE::GameContext &context, PE::MemoryArena arena,
			PE::Handle myHandle, float speed, Vector3 spawnPos,
			float networkPingInterval)
			: Component(context, arena, myHandle)
			, m_timeSpeed(speed)
			, m_time(0)
			, m_counter(0)
			, m_active(0)
			, m_networkPingTimer(0)
			, m_networkPingInterval(networkPingInterval)
			, m_overriden(false)
		{
			m_spawnPos = spawnPos;
		}

		void TankController::addDefaultComponents()
		{
			Component::addDefaultComponents();

			PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, TankController::do_UPDATE);

			// note: these event handlers will be registered only when one tank is activated as client tank (i.e. driven by client input on this machine)
			// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Throttle, TankController::do_Tank_Throttle);
			// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Turn, TankController::do_Tank_Turn);

		}
		void TankController::do_Tank_Throttle_Server(PE::Events::Event *pEvt)
		{
			assert(pEvt->isInstanceOf<Event_Tank_Throttle_From_Server>());

			Event_Tank_Throttle_From_Server *pRealEvent = (Event_Tank_Throttle_From_Server*)(pEvt);
			
			char buf[100];

			sprintf(buf, "\nIntended movement from server:(%f,%f,%f)", pRealEvent->m_relativeMove.m_x, pRealEvent->m_relativeMove.m_y, pRealEvent->m_relativeMove.m_z);
			OutputDebugStringA(buf);
			//////Walk Event for animation state machine - shabs
			//PE::Handle r("WALKEVENT", sizeof(Events::ClientTankAnimSM_Event_WALK));
			//Events::ClientTankAnimSM_Event_WALK *walkEvt = new(r) Events::ClientTankAnimSM_Event_WALK;

			//
			//Event_Tank_Throttle *pRealEvent = (Event_Tank_Throttle *)(pEvt);

			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

			float forwards_dist = pRealEvent->m_relativeMove.getZ();
			float up_dist = pRealEvent->m_relativeMove.getY();
			float right_dist = pRealEvent->m_relativeMove.getX();
			Vector3 current_pos = pFirstSN->m_base.getPos();
			Matrix4x4 m_next_pos = pFirstSN->m_base;
			m_next_pos.moveForward(forwards_dist);
			m_next_pos.moveRight(right_dist);
			m_next_pos.moveUp(up_dist);
			Vector3 next_pos = m_next_pos.getPos();

			////animation thing
			//pFirstSN->handleEvent(walkEvt);

			// Physics Testing
			Event_PHYSICS_PRE_UPDATE pPreUpdateEvt(current_pos, next_pos - current_pos, my_phys, pRealEvent->m_frameTime);
			next_pos = my_phys.postUpdate();
			next_pos.m_y -= my_phys.colliderSphere.radius; // Otherwise he tries to clip into the floor
			pFirstSN->m_base.setPos(next_pos);
			//
			//// Either one of these for memory leak. The former makes animation work for other player. Which is interesting.
			////PE::Events::EventQueueManager::Instance()->add(r, QT_GENERAL);
			//r.release();

			// Handle mesh triggers (end, lava, etc)
			if (my_phys.state == lava) {
				PE::Handle h("EVENT", sizeof(Event_Respawn));
				Event_Respawn *respawn_event = new(h) Event_Respawn;
				Event_Tank_Throttle *pRealEvent = (Event_Tank_Throttle *)(pEvt);
				PE::Events::EventQueueManager::Instance()->add(h, QT_GENERAL);
			}
			else if (my_phys.state == end) {
				CharacterControl::Events::Event_Server_Win evt(*m_pContext);
				//evt.m_transform = pFirstSN->m_base;

				ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
				pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);

			}

		}

		void TankController::do_Tank_Throttle(PE::Events::Event *pEvt)
		{

			////Walk Event for animation state machine - shabs
			PE::Handle r("WALKEVENT", sizeof(Events::ClientTankAnimSM_Event_WALK));
			Events::ClientTankAnimSM_Event_WALK *walkEvt = new(r) Events::ClientTankAnimSM_Event_WALK;

			
			Event_Tank_Throttle *pRealEvent = (Event_Tank_Throttle *)(pEvt);

			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

			float forwards_dist = pRealEvent->m_relativeMove.getZ();
			float up_dist = pRealEvent->m_relativeMove.getY();
			float right_dist = pRealEvent->m_relativeMove.getX();
			Vector3 current_pos = pFirstSN->m_base.getPos();
			Matrix4x4 m_next_pos = pFirstSN->m_base;
			m_next_pos.moveForward(forwards_dist);
			m_next_pos.moveRight(right_dist);
			m_next_pos.moveUp(up_dist);
			Vector3 next_pos = m_next_pos.getPos();
			//animation thing
			pFirstSN->handleEvent(walkEvt);

			// Physics Testing
			Event_PHYSICS_PRE_UPDATE pPreUpdateEvt(current_pos, next_pos - current_pos, my_phys, pRealEvent->m_frameTime);
			next_pos = my_phys.postUpdate();
			next_pos.m_y -= my_phys.colliderSphere.radius; // Otherwise he tries to clip into the floor
			pFirstSN->m_base.setPos(next_pos);
			
			// Either one of these for memory leak. The former makes animation work for other player. Which is interesting.
			//PE::Events::EventQueueManager::Instance()->add(r, QT_GENERAL);
			r.release();

			// Handle mesh triggers (end, lava, etc)
			if (my_phys.state == lava) {
				PE::Handle h("EVENT", sizeof(Event_Respawn));
				Event_Respawn *respawn_event = new(h) Event_Respawn;
				Event_Tank_Throttle *pRealEvent = (Event_Tank_Throttle *)(pEvt);
				PE::Events::EventQueueManager::Instance()->add(h, QT_GENERAL);
			}
			else if (my_phys.state == end) {
				CharacterControl::Events::Event_Server_Win evt(*m_pContext);
				//evt.m_transform = pFirstSN->m_base;

				ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
				pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);

			}

		}

		void TankController::do_Tank_Camera_Vert_Orbit(PE::Events::Event *pEvt) {
			Event_Tank_Camera_Vert_Orbit *pRealEvent = (Event_Tank_Camera_Vert_Orbit *)(pEvt);

			PE::Handle hCam = CameraManager::Instance()->getActiveCameraHandle();
			CameraSceneNode *pCamSN = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();

			// Handling actual rotation in TankController::do_UPDATE. Just set angle here
			currentCamOrbitAngle += pRealEvent->m_relativeRotate;

			// Clamp (there's a function for this somewhere....)
			static float _90Deg = 3.14159265359 / 2; // OK PI has to be somewhere too but I can't find it in PE right now
			currentCamOrbitAngle = currentCamOrbitAngle <= -_90Deg ? -_90Deg : currentCamOrbitAngle >= _90Deg ? _90Deg : currentCamOrbitAngle;

			char buf[100];
			sprintf(buf, "Camera angle: %f\n", currentCamOrbitAngle);
			OutputDebugStringA(buf);

		}

		void TankController::do_Tank_Turn(PE::Events::Event *pEvt)
		{
			Event_Tank_Turn *pRealEvent = (Event_Tank_Turn *)(pEvt);

			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

			//pFirstSN->m_base.turnUp(pRealEvent->m_relativeRotate.getY());
			pFirstSN->m_base.turnLeft(-pRealEvent->m_relativeRotate.getX());

		}
		void TankController::do_Stop(PE::Events::Event *pEvt) {
			PE::Handle r("STOPEVENT", sizeof(Events::ClientTankAnimSM_Event_STOP));
			Events::ClientTankAnimSM_Event_STOP *stopEvt = new(r) Events::ClientTankAnimSM_Event_STOP;
			//Event_Tank_STOP *pRealEvent = (Event_Tank_STOP *)(pEvt);
			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}
			//TankGameControls::wasSent = false;
			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
			//animation thing
			pFirstSN->handleEvent(stopEvt);
			r.release();
		}
		void TankController::do_Tank_Jump(PE::Events::Event *pEvt) {
			PE::Handle r("STOPEVENT", sizeof(Events::ClientTankAnimSM_Event_JUMP));
			Events::ClientTankAnimSM_Event_JUMP *jumpEvt = new(r) Events::ClientTankAnimSM_Event_JUMP;
			//Event_Tank_Jump *pRealEvent = (Event_Tank_Jump *)(pEvt);
			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}
			//Event_Tank_Jump* evt = (Event_Tank_Jump*)pEvt;
			jumpEvt->accMovement = my_phys.nextPosition;
			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
			//animation thing
			pFirstSN->handleEvent(jumpEvt);
			r.release();
		}
		void TankController::do_UPDATE(PE::Events::Event *pEvt)
		{
			PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);

			if (m_active)
			{
				m_time += pRealEvt->m_frameTime;
				m_networkPingTimer += pRealEvt->m_frameTime;
			}

			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();


			// ----- Camera Position and Rotation (Melron's bad maths) ----- //

			//static float x = 0.0f;
			//static float y = 6.0f;
			//static float z = -11.0f; // Artem's hard-coded camera pos (fixed behind player)

			float y, z; // x is always 0
			y = cameraToPlayerDist * sin(currentCamOrbitAngle);
			z = -cameraToPlayerDist * cos(currentCamOrbitAngle);

			// note we could have stored the camera reference in this object instead of searching for camera scene node
			if (CameraSceneNode *pCamSN = pFirstSN->getFirstComponent<CameraSceneNode>())
			{
				// We don't actually have to do this each frame. Could use a trigger when the camera is rotated up/down then change angle there
				Vector3 newCamPos = Vector3(0.0f, y, z);
				pCamSN->m_base.setPos(newCamPos);
				pCamSN->m_base.turnToVertical(-newCamPos);
			}


			if (!m_overriden)
			{
				/*
				if (m_time > 2.0f*PrimitiveTypes::Constants::c_Pi_F32)
				{
				m_time = 0;
				if (m_counter)
				{
				m_counter = 0;
				m_center = Vector2(0,0);
				}
				else
				{
				m_counter = 1;
				m_center = Vector2(10.0f, 0);
				}
				}

				Vector3 pos = Vector3(m_center.m_x, 0, m_center.m_y);
				pos.m_x += (float)cos(m_time) * 5.0f * (m_counter ? -1.0f : 1.0f);
				pos.m_z += (float)sin(m_time) * 5.0f;
				pos.m_y = 0;

				Vector3 fwrd;
				fwrd.m_x = -(float)sin(m_time)  * (m_counter ? -1.0f : 1.0f);
				fwrd.m_z = (float)cos(m_time);
				fwrd.m_y = 0;

				Vector3 right;
				right.m_x = (float)cos(m_time) * (m_counter ? -1.0f : 1.0f) * (m_counter ? -1.0f : 1.0f);
				right.m_z = (float)sin(m_time) * (m_counter ? -1.0f : 1.0f);
				right.m_y = 0;


				pFirstSN->m_base.setPos(m_spawnPos + pos);
				pFirstSN->m_base.setN(fwrd);
				pFirstSN->m_base.setU(right);
				*/
			}
			else
			{
				pFirstSN->m_base = m_transformOverride;
			}

			if (m_networkPingTimer > m_networkPingInterval)
			{
				// send client authoritative position event
				CharacterControl::Events::Event_MoveTank_C_to_S evt(*m_pContext);
				evt.m_transform = pFirstSN->m_base;
				evt.m_relativeMove = relative_movement_global;
				evt.m_frameTime = frame_time_global;
				ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
				pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);

				m_networkPingTimer = 0.0f;
			}
		}

		void TankController::overrideTransform(Matrix4x4 &t)
		{
			m_overriden = true;
			m_transformOverride = t;
		}

		void TankController::activate()
		{
			m_active = true;

			// this function is called on client tank. since this is client tank and we have client authoritative movement
			// we need to register event handling for movement here.
			// We have 6 tanks total. we activate tank controls controller (in GOM Addon) that will process input events into tank movement events
			// but we want only one tank to process those events. One way to do it is to dynamically add event handlers
			// to only one tank controller. this is what we do here.
			// another way to do this would be to only hae one tank controller, and have it grab one of tank scene nodes when activated
			PE_REGISTER_EVENT_HANDLER(Event_Tank_Throttle_From_Server, TankController::do_Tank_Throttle_Server);
			PE_REGISTER_EVENT_HANDLER(Event_Tank_Throttle, TankController::do_Tank_Throttle);
			PE_REGISTER_EVENT_HANDLER(Event_Tank_Turn, TankController::do_Tank_Turn);
			PE_REGISTER_EVENT_HANDLER(Event_Tank_Camera_Vert_Orbit, TankController::do_Tank_Camera_Vert_Orbit);
			PE_REGISTER_EVENT_HANDLER(Event_Respawn, TankController::do_Respawn);
			PE_REGISTER_EVENT_HANDLER(Event_Tank_STOP, TankController::do_Stop);
			PE_REGISTER_EVENT_HANDLER(Event_Tank_Jump, TankController::do_Tank_Jump);
			PE::Handle r("STOPEVENT", sizeof(Events::ClientTankAnimSM_Event_STOP));
			Events::ClientTankAnimSM_Event_STOP *stopEvt = new(r) Events::ClientTankAnimSM_Event_STOP;
			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			//create camera
			PE::Handle hCamera("Camera", sizeof(Camera));
			Camera *pCamera = new(hCamera) Camera(*m_pContext, m_arena, hCamera, hFisrtSN);
			pCamera->addDefaultComponents();
			CameraManager::Instance()->setCamera(CameraManager::VEHICLE, hCamera);

			CameraManager::Instance()->selectActiveCamera(CameraManager::VEHICLE);

			//disable default camera controls

			m_pContext->getDefaultGameControls()->setEnabled(false);
			m_pContext->get<CharacterControlContext>()->getSpaceShipGameControls()->setEnabled(false);
			//enable tank controls
			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
			m_pContext->get<CharacterControlContext>()->getTankGameControls()->setEnabled(true);
			pFirstSN->handleEvent(stopEvt);

		}

		void TankController::do_Respawn(PE::Events::Event *pEvt)
		{

			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}
			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
			pFirstSN->m_base.setPos(m_spawnPos);


		}



	}
}

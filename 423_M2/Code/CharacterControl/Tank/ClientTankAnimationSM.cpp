#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "ClientTankAnimationSM.h"
#include "ClientTank.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;


namespace CharacterControl {

	namespace Events {

		//PE_IMPLEMENT_CLASS1(ClientTankAnimSM_Event_WALK, Event);

		//PE_IMPLEMENT_CLASS1(ClientTankAnimSM_Event_WALK, Event);

		//PE_IMPLEMENT_CLASS1(ClientTankAnimSM_Event_RUN, Event);
	}

	namespace Components {

		PE_IMPLEMENT_CLASS1(ClientTankAnimationSM, DefaultAnimationSM);

		ClientTankAnimationSM::ClientTankAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) : DefaultAnimationSM(context, arena, hMyself)
		{
			m_curId = NONE;
			m_flagJump = false;
		}

		void ClientTankAnimationSM::addDefaultComponents()
		{
			DefaultAnimationSM::addDefaultComponents();

			PE_REGISTER_EVENT_HANDLER(Events::ClientTankAnimSM_Event_WALK, ClientTankAnimationSM::do_ClientTankAnimationSM_Event_WALK);
			PE_REGISTER_EVENT_HANDLER(Events::ClientTankAnimSM_Event_STOP, ClientTankAnimationSM::do_ClientTankAnimationSM_Event_STOP);
			PE_REGISTER_EVENT_HANDLER(Events::ClientTankAnimSM_Event_JUMP, ClientTankAnimationSM::do_ClientTankAnimationSM_Event_JUMP);
			PE_REGISTER_EVENT_HANDLER(Event_UPDATE, ClientTankAnimationSM::do_UPDATE);
		}

		

		void ClientTankAnimationSM::do_ClientTankAnimationSM_Event_WALK(PE::Events::Event *pEvt)
		{
			if (m_curId != ClientTankAnimationSM::WALK)
			{
				//char buffer[100];
				//sprintf(buffer, "REACHED INSIDE WALK EVENT");
				//OutputDebugStringA(buffer);
				m_curId = ClientTankAnimationSM::WALK;
				setAnimation(0, ClientTankAnimationSM::WALK,
					0, 0, 1, 1,
					PE::AnimSlotFlags::LOOPING);

			}
		}

		void ClientTankAnimationSM::do_ClientTankAnimationSM_Event_STOP(PE::Events::Event *pEvt)
		{
			if (m_curId != ClientTankAnimationSM::IDLE)
			{
				//char buffer[100];
				//sprintf(buffer, "REACHED INSIDE WALK EVENT");
				//OutputDebugStringA(buffer);
				m_curId = ClientTankAnimationSM::IDLE;
				setAnimation(0, ClientTankAnimationSM::IDLE,
					0, 0, 1, 1,
					PE::AnimSlotFlags::LOOPING);
			}
		}

		void ClientTankAnimationSM::do_ClientTankAnimationSM_Event_JUMP(PE::Events::Event *pEvt)
		{
			ClientTankAnimSM_Event_JUMP* evt = (ClientTankAnimSM_Event_JUMP*)pEvt;
			if (m_curId != ClientTankAnimationSM::JUMP)
			{
				m_curId = ClientTankAnimationSM::JUMP;
				if (evt->accMovement.m_z>=-1.0f && evt->accMovement.m_z <= 0.0f) {
					JumpSlot = setAnimation(0, ClientTankAnimationSM::JUMP,
						0, 0, 1, 1,
						PE::AnimSlotFlags::STAY_ON_ANIMATION_END);
					m_flagStay = true;
				}
				else {
					JumpSlot = setAnimation(0, ClientTankAnimationSM::JUMP,
						0, 0, 1, 1,
						PE::AnimSlotFlags::LOOPING);
					m_flagJump = true;
				}
				
				
				
			}
		}
		
		void ClientTankAnimationSM::do_UPDATE(PE::Events::Event *pEvt) {
			PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
			//OutputDebugStringA("update in amination\n");
			if (m_flagJump)
			{
				if (JumpSlot->m_numFrames - JumpSlot->m_framesLeft <= 1) {
					setAnimation(0, ClientTankAnimationSM::JUMPEND,
						0, 0, 1, 1,
						PE::AnimSlotFlags::LOOPING);
					m_flagJump = false;
				}
			}
			else if (m_flagStay)
			{
					setAnimation(0, ClientTankAnimationSM::JUMPSTAND,
						0, 0, 1, 1,
						PE::AnimSlotFlags::LOOPING);
					m_flagStay = false;
				
			}
		}


	}
}





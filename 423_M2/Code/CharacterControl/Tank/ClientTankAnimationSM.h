#ifndef _PE_CLIENT_TANK_ANIMATION_SM_H_
#define _PE_CLIENT_TANK_ANIMATION_SM_H_


#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/DefaultAnimationSM.h"


#include "../Events/Events.h"

namespace CharacterControl {

	// events that can be sent to this state machine
	namespace Events
	{

		// sent by movement state machine when character has to walk
		/*struct ClientTankAnimSM_Event_STOP : public PE::Events::Event {

			PE_DECLARE_CLASS(ClientTankAnimSM_Event_STOP);

			ClientTankAnimSM_Event_STOP() {}
		};*/

		

	};

	namespace Components {

		struct ClientTankAnimationSM : public PE::Components::DefaultAnimationSM
		{
			PE_DECLARE_CLASS(ClientTankAnimationSM);

			enum AnimId
			{
				NONE = -1,
				WALK = 17,  //Soldier: 18
				IDLE = 1,	//Soldier: 20
				JUMP = 13,
				JUMPEND = 11,
				JUMPSTAND = 7, 
			};

			ClientTankAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);

			// event handling
			virtual void addDefaultComponents();
			
			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_ClientTankAnimationSM_Event_WALK)
				virtual void do_ClientTankAnimationSM_Event_WALK(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_ClientTankAnimationSM_Event_STOP)
				virtual void do_ClientTankAnimationSM_Event_STOP(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_ClientTankAnimationSM_Event_JUMP)
				virtual void do_ClientTankAnimationSM_Event_JUMP(PE::Events::Event *pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
				virtual void do_UPDATE(PE::Events::Event *pEvt);
			
			AnimId m_curId;
			bool m_flagJump;
			bool m_flagStay;
			PE::AnimationSlot* JumpSlot;
		};

	};
};


#endif




#ifndef _CHARACTER_CONTROL_SOLDIER_NPC_
#define _CHARACTER_CONTROL_SOLDIER_NPC_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Physics/PhysicsComponent.h"


#include "../Events/Events.h"

namespace CharacterControl{

namespace Components {

struct SoldierNPC : public PE::Components::Component
{
	PE_DECLARE_CLASS(SoldierNPC);

	SoldierNPC(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreateSoldierNPC *pEvt);

	virtual void addDefaultComponents();

	// Target information
	char m_isTargetName[32];
	char m_hasTargetName[32];

	PhysicsComponent my_physics_component;

	Matrix4x4 m_pos;
};
}; // namespace Components
}; // namespace CharacterControl
#endif


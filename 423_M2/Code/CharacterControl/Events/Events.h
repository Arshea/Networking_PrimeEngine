#ifndef _CHARACTER_CONTROL_EVENTS_
#define _CHARACTER_CONTROL_EVENTS_

#include "PrimeEngine/Events/StandardEvents.h"

namespace CharacterControl
{
namespace Events
{
struct Event_CreateSoldierNPC : public PE::Events::Event_CREATE_MESH
{
	PE_DECLARE_CLASS(Event_CreateSoldierNPC);

	Event_CreateSoldierNPC(int &threadOwnershipMask): PE::Events::Event_CREATE_MESH(threadOwnershipMask){}
	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	int m_npcType;
	char m_gunMeshName[64];
	char m_gunMeshPackage[64];
	char m_patrolWayPoint[32];
	char m_isTargetName[32];
	char m_getsTargetName[32];
};

struct Event_MoveTank_C_to_S : public PE::Events::Event, public PE::Networkable
{
	PE_DECLARE_CLASS(Event_MoveTank_C_to_S);
	PE_DECLARE_NETWORKABLE_CLASS

	Event_MoveTank_C_to_S(PE::GameContext &context);
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);


	Matrix4x4 m_transform;

	PrimitiveTypes::Float32 m_frameTime;
	Vector3 m_relativeMove;
};


struct Event_MoveTank_S_to_C : public Event_MoveTank_C_to_S
{
	PE_DECLARE_CLASS(Event_MoveTank_S_to_C);
	PE_DECLARE_NETWORKABLE_CLASS

	Event_MoveTank_S_to_C(PE::GameContext &context);
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

	int m_clientTankId;
};

struct Event_Tank_Throttle_From_Server : public Event_MoveTank_C_to_S {
	PE_DECLARE_CLASS(Event_Tank_Throttle_From_Server);
	PE_DECLARE_NETWORKABLE_CLASS

	Event_Tank_Throttle_From_Server(PE::GameContext &context);
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);

	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);


};
//////////////////////////////////////////////////////////////////////////
//////////////////////// WIN/LOSE EVENTS ////////////////////////////////
////////////////////////////////////////////////////////////////////////

struct Event_Server_Win : public PE::Events::Event, public PE::Networkable
{
	PE_DECLARE_CLASS(Event_Server_Win);
	PE_DECLARE_NETWORKABLE_CLASS

		Event_Server_Win(PE::GameContext &context);

	//not sure about this bit yet
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);


	//Matrix4x4 m_transform;
};


struct Event_Client_Lose : public Event_Server_Win
{
	PE_DECLARE_CLASS(Event_Client_Lose);
	PE_DECLARE_NETWORKABLE_CLASS

		Event_Client_Lose(PE::GameContext &context);

	//Not sure about this bit yet
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

	int m_clientTankId;
};

struct Event_Client_Win : public Event_Server_Win
{
	PE_DECLARE_CLASS(Event_Client_Win);
	PE_DECLARE_NETWORKABLE_CLASS

		Event_Client_Win(PE::GameContext &context);

	//Not sure about this bit yet
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

	int m_clientTankId;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



// tank input controls

struct Event_Tank_Throttle : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Throttle);

	Event_Tank_Throttle(){}
	virtual ~Event_Tank_Throttle(){}

	PrimitiveTypes::Float32 m_frameTime;
	Vector3 m_relativeMove;
};

struct Event_Tank_Camera_Vert_Orbit : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Camera_Vert_Orbit);

	Event_Tank_Camera_Vert_Orbit() {}
	virtual ~Event_Tank_Camera_Vert_Orbit() {}

	float m_relativeRotate;
};

struct Event_Tank_Turn : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Turn);

	Event_Tank_Turn(){}
	virtual ~Event_Tank_Turn(){}

	Vector3 m_relativeRotate;
};

struct Event_Tank_Jump : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Jump);

	Event_Tank_Jump() {}
	virtual ~Event_Tank_Jump() {}
	Vector3 accMovement;
};

struct Event_Tank_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_STOP);

	Event_Tank_STOP() {}
	virtual ~Event_Tank_STOP() {}
};

struct Event_Respawn : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Respawn);

	Event_Respawn() {}
	virtual ~Event_Respawn() {}

};

//////////////////////////////////////////////////////////////////////////
//////////////////////// ANIMATION EVENTS ///////////////////////////////
////////////////////////////////////////////////////////////////////////
struct ServerTankAnimSM_Event_WALK : public PE::Events::Event, public PE::Networkable
{
	PE_DECLARE_CLASS(ServerTankAnimSM_Event_WALK);
	PE_DECLARE_NETWORKABLE_CLASS

		ServerTankAnimSM_Event_WALK(PE::GameContext &context);

	//not sure about this bit yet
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);


	//Matrix4x4 m_transform;
};
struct Event_Client_Walk : public ServerTankAnimSM_Event_WALK
{
	PE_DECLARE_CLASS(Event_Client_Walk);
	PE_DECLARE_NETWORKABLE_CLASS

		Event_Client_Walk(PE::GameContext &context);

	//Not sure about this bit yet
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

	int m_clientTankId;
};
//////////////////////////////////////////////////////////
struct ClientTankAnimSM_Event_WALK : public PE::Events::Event {
	PE_DECLARE_CLASS(ClientTankAnimSM_Event_WALK);

	ClientTankAnimSM_Event_WALK() {}
	virtual ~ClientTankAnimSM_Event_WALK() {}
};
struct ClientTankAnimSM_Event_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(ClientTankAnimSM_Event_STOP);
	ClientTankAnimSM_Event_STOP() {}
	virtual ~ClientTankAnimSM_Event_STOP() {}
};
struct ClientTankAnimSM_Event_JUMP : public PE::Events::Event {
	PE_DECLARE_CLASS(ClientTankAnimSM_Event_JUMP);
	ClientTankAnimSM_Event_JUMP() {}
	virtual ~ClientTankAnimSM_Event_JUMP() {}
	//MeshTypes ground;
	Vector3 accMovement;
};
}; // namespace Events
}; // namespace CharacterControl

#endif

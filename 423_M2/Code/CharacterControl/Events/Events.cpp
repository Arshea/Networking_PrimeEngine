#include "PrimeEngine/PrimeEngineIncludes.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "Events.h"

using namespace PE;
namespace CharacterControl {
namespace Events{

PE_IMPLEMENT_CLASS1(Event_CreateSoldierNPC, PE::Events::Event_CREATE_SKELETON);

void Event_CreateSoldierNPC::SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM)
{
	static const struct luaL_Reg l_Event_CreateSoldierNPC[] = {
		{"Construct", l_Construct},
		{NULL, NULL} // sentinel
	};

	// register the functions in current lua table which is the table for Event_CreateSoldierNPC
	luaL_register(luaVM, 0, l_Event_CreateSoldierNPC);
}

int Event_CreateSoldierNPC::l_Construct(lua_State* luaVM)
{
    PE::Handle h("EVENT", sizeof(Event_CreateSoldierNPC));
	
	// get arguments from stack
	int numArgs, numArgsConst;
	numArgs = numArgsConst = 21;

	PE::GameContext *pContext = (PE::GameContext*)(lua_touserdata(luaVM, -numArgs--));

	// this function should only be called frm game thread, so we can use game thread thread owenrship mask
	Event_CreateSoldierNPC *pEvt = new(h) Event_CreateSoldierNPC(pContext->m_gameThreadThreadOwnershipMask);

	const char* name = lua_tostring(luaVM, -numArgs--);
	const char* package = lua_tostring(luaVM, -numArgs--);

	const char* gunMeshName = lua_tostring(luaVM, -numArgs--);
	const char* gunMeshPackage = lua_tostring(luaVM, -numArgs--);

	float positionFactor = 1.0f / 100.0f;

	Vector3 playerPos, u, v, n;
	playerPos.m_x = (float)lua_tonumber(luaVM, -numArgs--) * positionFactor;
	playerPos.m_y = (float)lua_tonumber(luaVM, -numArgs--) * positionFactor;
	playerPos.m_z = (float)lua_tonumber(luaVM, -numArgs--) * positionFactor;

	u.m_x = (float)lua_tonumber(luaVM, -numArgs--); u.m_y = (float)lua_tonumber(luaVM, -numArgs--); u.m_z = (float)lua_tonumber(luaVM, -numArgs--);
	v.m_x = (float)lua_tonumber(luaVM, -numArgs--); v.m_y = (float)lua_tonumber(luaVM, -numArgs--); v.m_z = (float)lua_tonumber(luaVM, -numArgs--);
	n.m_x = (float)lua_tonumber(luaVM, -numArgs--); n.m_y = (float)lua_tonumber(luaVM, -numArgs--); n.m_z = (float)lua_tonumber(luaVM, -numArgs--);

	pEvt->m_peuuid = LuaGlue::readPEUUID(luaVM, -numArgs--);

	const char* wayPointName = NULL;

	if (!lua_isnil(luaVM, -numArgs))
	{
		// have patrol waypoint name
		wayPointName = lua_tostring(luaVM, -numArgs--);
	}
	else
		// ignore
		numArgs--;

	const char* isTargetName = NULL;
	const char* getsTargetName = NULL;

	if (!lua_isnil(luaVM, -numArgs))
	{
		isTargetName = lua_tostring(luaVM, -numArgs--);
	}
	else
		// ignore
		numArgs--;

	if (!lua_isnil(luaVM, -numArgs))
	{
		getsTargetName = lua_tostring(luaVM, -numArgs--);
	}
	else
		// ignore
		numArgs--;


	// set data values before popping memory off stack
	StringOps::writeToString(name, pEvt->m_meshFilename, 255);
	StringOps::writeToString(package, pEvt->m_package, 255);

	StringOps::writeToString(gunMeshName, pEvt->m_gunMeshName, 64);
	StringOps::writeToString(gunMeshPackage, pEvt->m_gunMeshPackage, 64);
	StringOps::writeToString(wayPointName, pEvt->m_patrolWayPoint, 32);

	StringOps::writeToString(isTargetName, pEvt->m_isTargetName, 32);

	OutputDebugStringA("Adding isTargetName: ");
	OutputDebugStringA(isTargetName);
	OutputDebugStringA("\n");

	StringOps::writeToString(getsTargetName, pEvt->m_getsTargetName, 32);

	lua_pop(luaVM, numArgsConst); //Second arg is a count of how many to pop

	pEvt->hasCustomOrientation = true;
	pEvt->m_pos = playerPos;
	pEvt->m_u = u;
	pEvt->m_v = v;
	pEvt->m_n = n;

	LuaGlue::pushTableBuiltFromHandle(luaVM, h); 

	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Server event added for winning///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
PE_IMPLEMENT_CLASS1(Event_Server_Win, Event);

Event_Server_Win::Event_Server_Win(PE::GameContext &context)
: Networkable(context, this)
{

}

void *Event_Server_Win::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_Server_Win *pEvt = new (arena) Event_Server_Win(context);
	return pEvt;
}

//Not sure about this bit yet

int Event_Server_Win::packCreationData(char *pDataStream)
{
	return 0;
}

int Event_Server_Win::constructFromStream(char *pDataStream)
{
	int read = 0;
	//read += PE::Components::StreamManager::ReadMatrix4x4(&pDataStream[read], m_transform);
	return read;
}
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
////////////////////Client win events/////////////////////////////////
PE_IMPLEMENT_CLASS1(Event_Client_Lose, Event_Server_Win);

Event_Client_Lose::Event_Client_Lose(PE::GameContext &context)
	: Event_Server_Win(context)
{

}

void *Event_Client_Lose::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_Client_Lose *pEvt = new (arena) Event_Client_Lose(context);
	return pEvt;
}

int Event_Client_Lose::packCreationData(char *pDataStream)
{
	int size = 0;
	size += Event_Server_Win::packCreationData(&pDataStream[size]);
	//size += PE::Components::StreamManager::WriteInt32(m_clientTankId, &pDataStream[size]);
	return size;
}

int Event_Client_Lose::constructFromStream(char *pDataStream)
{
	int read = 0;
	read += Event_Server_Win::constructFromStream(&pDataStream[read]);
	//read += PE::Components::StreamManager::ReadInt32(&pDataStream[read], m_clientTankId);
	return read;
}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
PE_IMPLEMENT_CLASS1(Event_Client_Win, Event_Server_Win);

Event_Client_Win::Event_Client_Win(PE::GameContext &context)
	: Event_Server_Win(context)
{

}

void *Event_Client_Win::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_Client_Win *pEvt = new (arena) Event_Client_Win(context);
	return pEvt;
}

int Event_Client_Win::packCreationData(char *pDataStream)
{
	int size = 0;
	size += Event_Server_Win::packCreationData(&pDataStream[size]);
	//size += PE::Components::StreamManager::WriteInt32(m_clientTankId, &pDataStream[size]);
	return size;
}

int Event_Client_Win::constructFromStream(char *pDataStream)
{
	int read = 0;
	read += Event_Server_Win::constructFromStream(&pDataStream[read]);
	//read += PE::Components::StreamManager::ReadInt32(&pDataStream[read], m_clientTankId);
	return read;
}

//////////////////////////////////////////////////////////////////////////
PE_IMPLEMENT_CLASS1(Event_MoveTank_C_to_S, Event);

Event_MoveTank_C_to_S::Event_MoveTank_C_to_S(PE::GameContext &context)
	: Networkable(context, this)
{

}

void *Event_MoveTank_C_to_S::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_MoveTank_C_to_S *pEvt = new (arena) Event_MoveTank_C_to_S(context);
	return pEvt;
}

int Event_MoveTank_C_to_S::packCreationData(char *pDataStream)
{
	int size = 0;
	size += PE::Components::StreamManager::WriteMatrix4x4(m_transform, &pDataStream[size]);
	size += PE::Components::StreamManager::WriteVector3(m_relativeMove, &pDataStream[size]);
	size += PE::Components::StreamManager::WriteFloat32(m_frameTime, &pDataStream[size]);
	return size;

}

int Event_MoveTank_C_to_S::constructFromStream(char *pDataStream)
{
	int read = 0;
	read += PE::Components::StreamManager::ReadMatrix4x4(&pDataStream[read], m_transform);
	read += PE::Components::StreamManager::ReadVector3(&pDataStream[read], m_relativeMove);
	read += PE::Components::StreamManager::ReadFloat32(&pDataStream[read], m_frameTime);
	return read;
}


PE_IMPLEMENT_CLASS1(Event_MoveTank_S_to_C, Event_MoveTank_C_to_S);

Event_MoveTank_S_to_C::Event_MoveTank_S_to_C(PE::GameContext &context)
: Event_MoveTank_C_to_S(context)
{

}

void *Event_MoveTank_S_to_C::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_MoveTank_S_to_C *pEvt = new (arena) Event_MoveTank_S_to_C(context);
	return pEvt;
}

int Event_MoveTank_S_to_C::packCreationData(char *pDataStream)
{
	int size = 0;
	size += Event_MoveTank_C_to_S::packCreationData(&pDataStream[size]);
	size += PE::Components::StreamManager::WriteInt32(m_clientTankId, &pDataStream[size]);
	return size;
}

int Event_MoveTank_S_to_C::constructFromStream(char *pDataStream)
{
	int read = 0;
	read += Event_MoveTank_C_to_S::constructFromStream(&pDataStream[read]);
	read += PE::Components::StreamManager::ReadInt32(&pDataStream[read], m_clientTankId);
	return read;
}


PE_IMPLEMENT_CLASS1(Event_Tank_Throttle_From_Server, Event_MoveTank_C_to_S);

Event_Tank_Throttle_From_Server::Event_Tank_Throttle_From_Server(PE::GameContext &context)
	: Event_MoveTank_C_to_S(context)
{

}

void *Event_Tank_Throttle_From_Server::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_Tank_Throttle_From_Server *pEvt = new (arena) Event_Tank_Throttle_From_Server(context);
	return pEvt;
}

int Event_Tank_Throttle_From_Server::packCreationData(char *pDataStream)
{
	int size = 0;
	size += Event_MoveTank_C_to_S::packCreationData(&pDataStream[size]);
	return size;
}

int Event_Tank_Throttle_From_Server::constructFromStream(char *pDataStream)
{
	int read = 0;
	read += Event_MoveTank_C_to_S::constructFromStream(&pDataStream[read]);
	return read;
}


PE_IMPLEMENT_CLASS1(Event_Tank_Throttle, Event);

PE_IMPLEMENT_CLASS1(Event_Tank_Turn, Event);

PE_IMPLEMENT_CLASS1(Event_Respawn, Event);

PE_IMPLEMENT_CLASS1(Event_Tank_Camera_Vert_Orbit, Event);

PE_IMPLEMENT_CLASS1(Event_Tank_STOP, Event);

PE_IMPLEMENT_CLASS1(Event_Tank_Jump, Event);

//////////////////////////////////////////////////////////////////////////
//////////////////////// ANIMATION EVENTS ///////////////////////////////
////////////////////////////////////////////////////////////////////////
PE_IMPLEMENT_CLASS1(ServerTankAnimSM_Event_WALK, Event);

ServerTankAnimSM_Event_WALK::ServerTankAnimSM_Event_WALK(PE::GameContext &context)
	: Networkable(context, this)
{

}

void *ServerTankAnimSM_Event_WALK::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	ServerTankAnimSM_Event_WALK *pEvt = new (arena) ServerTankAnimSM_Event_WALK(context);
	return pEvt;
}

//Not sure about this bit yet

int ServerTankAnimSM_Event_WALK::packCreationData(char *pDataStream)
{
	return 0;
}

int ServerTankAnimSM_Event_WALK::constructFromStream(char *pDataStream)
{
	int read = 0;
	//read += PE::Components::StreamManager::ReadMatrix4x4(&pDataStream[read], m_transform);
	return read;
}

PE_IMPLEMENT_CLASS1(Event_Client_Walk, ServerTankAnimSM_Event_WALK);

Event_Client_Walk::Event_Client_Walk(PE::GameContext &context)
	: ServerTankAnimSM_Event_WALK(context)
{

}

void *Event_Client_Walk::FactoryConstruct(PE::GameContext& context, PE::MemoryArena arena)
{
	Event_Client_Walk *pEvt = new (arena) Event_Client_Walk(context);
	return pEvt;
}

int Event_Client_Walk::packCreationData(char *pDataStream)
{
	int size = 0;
	size += ServerTankAnimSM_Event_WALK::packCreationData(&pDataStream[size]);
	//size += PE::Components::StreamManager::WriteInt32(m_clientTankId, &pDataStream[size]);
	return size;
}

int Event_Client_Walk::constructFromStream(char *pDataStream)
{
	int read = 0;
	read += ServerTankAnimSM_Event_WALK::constructFromStream(&pDataStream[read]);
	//read += PE::Components::StreamManager::ReadInt32(&pDataStream[read], m_clientTankId);
	return read;
}
//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

PE_IMPLEMENT_CLASS1(ClientTankAnimSM_Event_WALK, Event);
PE_IMPLEMENT_CLASS1(ClientTankAnimSM_Event_STOP, Event);
PE_IMPLEMENT_CLASS1(ClientTankAnimSM_Event_JUMP, Event);
};
};


#ifndef __PrimeEngineConnectionManager_H__
#define __PrimeEngineConnectionManager_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>
#include <vector>

// Inter-Engine includes

#include "../Events/Component.h"
#include "../../../GlobalConfig/GlobalConfig.h"

extern "C"
{
#include "../../luasocket_dist/src/socket.h"
};


// Sibling/Children includes
#include "PrimeEngine/Networking/NetworkContext.h"
#include "Packet.h"

namespace PE {
namespace Components {

struct ConnectionManager : public Component
{
	PE_DECLARE_CLASS(ConnectionManager);

	// Constructor -------------------------------------------------------------
	ConnectionManager(PE::GameContext &context, PE::MemoryArena arena, PE::NetworkContext &netContext, Handle hMyself);

	virtual ~ConnectionManager();

	// note: there is no connecting state. connecting is done by network mananger
	// once this connection manager disconnects, it never reconnects on its own

	enum EConnectionManagerState
	{
		ConnectionManagerState_Disconnected = 0,
		ConnectionManagerState_Connected,
		ConnectionManagerState_Count
	};

	// Methods -----------------------------------------------------------------
	virtual void initializeConnected(t_socket sock, sockaddr_in recvaddr, sockaddr_in sendaddr);

	void sendPacket(Packet *pPacket, TransmissionRecord *pTransmissionRecord);
	
	void receivePackets();
	bool connected() const {return m_state == ConnectionManagerState_Connected;}
	void disconnect();
	void debugRender(int &threadOwnershipMask, float xoffset /* = 0*/, float yoffset /* = 0*/);
	// Component ------------------------------------------------------------
	virtual void addDefaultComponents();

	// Individual events -------------------------------------------------------
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
	virtual void do_UPDATE(Events::Event *pEvt);

	// Loading -----------------------------------------------------------------

#if 0 // template
	//////////////////////////////////////////////////////////////////////////
	// Skin Lua Interface
	//////////////////////////////////////////////////////////////////////////
	//
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);
	//
	static int l_clientConnectToTCPServer(lua_State *luaVM);
	//
	//////////////////////////////////////////////////////////////////////////
#endif
	//////////////////////////////////////////////////////////////////////////
	// Member variables 
	//////////////////////////////////////////////////////////////////////////
	struct AckSim
	{
		bool m_delivered;
		int m_packetId;
	};
	std::vector<AckSim> m_ackSimulation;

	PE::NetworkContext *m_pNetContext;

	/*luasocket::*/t_socket m_sock; // udp connection socket
	EConnectionManagerState m_state;

	sockaddr_in m_recvaddr;
	sockaddr_in m_sendaddr;

	int m_bytesNeededForNextPacket;
	bool m_messageToSend;
	int m_bytesBuffered;
	char m_buffer[PE_SOCKET_RECEIVE_BUFFER_SIZE];

	int num_packets_dropped;

};
}; // namespace Components
}; // namespace PE
#endif

#ifndef _CHARACTER_CONTROL_FLYER_
#define _CHARACTER_CONTROL_FLYER_

#include "PrimeEngine/Events/Component.h"


#include "../Events/Events.h"
#include "PrimeEngine/Scene/CameraManager.h"


#ifdef _XBOX
// NUI testing
#include "Atg/AtgJointFilter.h"
#include "Atg/AtgNuiJointConverter.h"
#include "Atg/AtgNuiCommon.h"
#include "Atg/AtgAvatarRenderer.h"
#endif

namespace CharacterControl{

namespace Components {

struct Flyer : public PE::Components::Component
{
	PE_DECLARE_CLASS(Flyer);

	Flyer(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, int &threadOwnershipMask);

	virtual void addDefaultComponents() ;

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
	virtual void do_UPDATE(PE::Events::Event *pEvt);

	// NUI testing
#ifdef _XBOX 
	ATG::FilterDoubleExponential m_JointFilter;
	HANDLE m_hFrameEndEvent;
	NUI_SKELETON_FRAME m_SkeletonFrame;
	XAVATAR_SKELETON_POSE_JOINT m_AvatarJointPoseNuiMapperConstrained[ XAVATAR_MAX_SKELETON_JOINTS ]; 
	IXAvatarNuiMapper*          m_pAvatarNuiMapperConstrained;

	ATG::NuiJointConverter*     m_pNuiJointConverterConstrained;       // Joint converter that uses joint constraints
	PE::Components::SceneNode *m_pCamSN;
	int m_framesWithNoData;
	int m_framesWithData;

	Vector3 m_lastLeftPos, m_lastRightPos;
	Vector3 m_speed;
	PE::Components::SceneNode *m_pNuiSN;
	PE::Components::SceneNode *m_sceneNodes[XAVATAR_MAX_SKELETON_JOINTS];
#endif

	PE::Components::CameraManager::CameraType m_prevCameraType;
};
}; // namespace Components
}; // namespace CharacterControl
#endif


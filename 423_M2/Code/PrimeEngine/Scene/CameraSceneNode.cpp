#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"


// For camera frustums
#include "PrimeEngine/Scene/DebugRenderer.h"

#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);

CameraSceneNode::CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
{
	m_near = 0.05f;
	m_far = 2000.0f;
}
void CameraSceneNode::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
}

void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt)
{
	Handle hParentSN = getFirstParentByType<SceneNode>();
	if (hParentSN.isValid())
	{
		Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
		m_worldTransform = parentTransform * m_base;
	}
	
	Matrix4x4 &mref_worldTransform = m_worldTransform;

	Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]);
	Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]);
	Vector3 target = pos + n;
	Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

	m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

	m_worldTransform2 = mref_worldTransform;

	m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

	Vector3 pos2 = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
	Vector3 n2 = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
	Vector3 target2 = pos2 + n2;
	Vector3 up2 = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

	m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);
    
    PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
    
    PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
    if (aspect < 1.0f)
    {
        //ios portrait view
        static PrimitiveTypes::Float32 factor = 0.5f;
        verticalFov *= factor;
    }

	m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, 
		aspect,
		m_near, m_far);
	
	SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);

	Frustum c_frustum(m_near, m_far, aspect, verticalFov);
	c_frustum.setCamDef(pos, target, up);
	frustum = c_frustum;
	//Vector3 *testPrint = frustum.debugPrintVisual();
	
	//showFrustumDebug(frustum);

}

void CameraSceneNode::showFrustumDebug(Frustum frustum) {
	Vector3 colorFront = { 1.0f, 0.25f, 1.0f };
	Vector3 colorBack = { 0.5f, 0.0f, 0.25f };
	Vector3 testPrint[24];

	for (int i = 1; i < 20; i += 2) {
		testPrint[i] = colorFront;
	}
	testPrint[21] = colorBack;
	testPrint[23] = colorBack;

	testPrint[0] = frustum.ntl;
	testPrint[2] = frustum.nbr;
	testPrint[4] = frustum.ntl;
	testPrint[6] = frustum.ntr;
	testPrint[8] = frustum.ntr;
	testPrint[10] = frustum.nbr;
	testPrint[12] = frustum.nbr;
	testPrint[14] = frustum.nbl;
	testPrint[16] = frustum.nbl;
	testPrint[18] = frustum.ntl;
	testPrint[20] = frustum.ftr;
	testPrint[22] = frustum.fbl;

	DebugRenderer::Instance()->createLineMesh(false, Matrix4x4(), &testPrint[0].m_x, 12, 1.0f);
}

}; // namespace Components
}; // namespace PE

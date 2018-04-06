#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "Flyer.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/MeshInstance.h"

using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl{
namespace Components {

PE_IMPLEMENT_CLASS1(Flyer, Component);

Flyer::Flyer(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, int &threadOwnershipMask) 
: Component(context, arena, hMyself)
, m_prevCameraType(CameraManager::CameraType_Count)
{
	// NUI testing
#ifdef _XBOX
	// Could also try for a bit more smoothing ( 0.25f, 0.25f, 0.25f, 0.03f, 0.05f );
	m_JointFilter.Init( 0.5f, 0.5f, 0.5f, 0.05f, 0.05f );

	// create event which will be signaled when frame processing ends
	m_hFrameEndEvent = CreateEvent( NULL,
		FALSE,  // auto-reset
		FALSE,  // create unsignaled
		"NuiFrameEndEvent" );

	if ( !m_hFrameEndEvent )
	{
		ATG_PrintError( "Failed to create NuiFrameEndEvent\n" );
		return;
		// return E_FAIL;
	}

	HRESULT hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_SKELETON |
		NUI_INITIALIZE_FLAG_USES_COLOR |
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX,
		NUI_INITIALIZE_DEFAULT_HARDWARE_THREAD );

	if( FAILED( hr ))
	{
		ATG::NuiPrintError( hr, "NuiInitialize" );
		return;
		// return E_FAIL;
	}
	
	// register frame end event with NUI
	hr = NuiSetFrameEndEvent( m_hFrameEndEvent, 0 );
	if( FAILED(hr) )
	{
		ATG::NuiPrintError( hr, "NuiSetFrameEndEvent" );
		return;
		// return E_FAIL;
	}

	/*
	// Open the color stream
	hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 1, NULL, &m_hImage );
	if( FAILED (hr) )
	{
		ATG::NuiPrintError( hr, "NuiImageStreamOpen" );
		return E_FAIL;
	}

	// Open the depth stream
	hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, 0, 1, NULL, &m_hDepth );
	if( FAILED (hr) )
	{
		ATG::NuiPrintError( hr, "NuiImageStreamOpen" );
		return E_FAIL;
	}
	*/
	hr = NuiSkeletonTrackingEnable( NULL, 0 );
	if( FAILED( hr ))
	{
		ATG::NuiPrintError( hr, "NuiSkeletonTrackingEnable" );
	}

	m_pNuiJointConverterConstrained = new ATG::NuiJointConverter();
	if( m_pNuiJointConverterConstrained == NULL )
	{
		// return E_FAIL;
		return;
	}

	m_pNuiJointConverterConstrained->AddDefaultConstraints();
	
	Handle hSN("SceneNode", sizeof(SceneNode));
	m_pNuiSN = new(hSN) SceneNode(context, arena, hSN);
	m_pNuiSN->addDefaultComponents();
	m_pNuiSN->m_base.setPos(Vector3(0.0f, 0, 25.0f));
	m_pNuiSN->m_base.turnRight(1.2f * 3.1415f);
	RootSceneNode::Instance()->addComponent(hSN);
	for (int i = 0; i < XAVATAR_MAX_SKELETON_JOINTS; i++)
	{
		PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
		MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("box.x_main_mesh.mesha", "Default", threadOwnershipMask);
		
		Handle hSN("SceneNode", sizeof(SceneNode));
		m_sceneNodes[i] = new(hSN) SceneNode(context, arena, hSN);
		m_sceneNodes[i]->addDefaultComponents();
		m_sceneNodes[i]->addComponent(hMeshInstance);
		m_pNuiSN->addComponent(hSN);

		if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_SHOULDER_RIGHT)
		{
			PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
			pMeshInstance->addDefaultComponents();
			pMeshInstance->initFromFile("wings.x_rwing_mesh.mesha", "City", threadOwnershipMask);
			
			m_sceneNodes[i]->addComponent(hMeshInstance);
		}

		if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_SHOULDER_LEFT)
		{
			
			PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

			pMeshInstance->addDefaultComponents();
			pMeshInstance->initFromFile("wings.x_lwing_mesh.mesha", "City", threadOwnershipMask);

			m_sceneNodes[i]->addComponent(hMeshInstance);
		}
	}
	{
			// put a camera here

		PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
		MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

		pMeshInstance->addDefaultComponents();
		pMeshInstance->initFromFile("box.x_main_mesh.mesha", "Default", threadOwnershipMask);

		// we put camera in a scene node so we can rotate the camera within that scene node, but we could have also just added camera on its own
		Handle hSN("SceneNode", sizeof(SceneNode));
		m_pCamSN = new(hSN) SceneNode(context, arena,hSN);
		m_pCamSN->addDefaultComponents();
		m_pCamSN->addComponent(hMeshInstance);
		m_pNuiSN->addComponent(hSN);
		m_pCamSN->m_base.setPos(Vector3(0, +1.0f, +1.5f));

		Handle hDebugCamera("Camera", sizeof(Camera));
		Camera *debugCamera = new(hDebugCamera) Camera(context, arena, hDebugCamera, hSN);
		debugCamera->addDefaultComponents();
		CameraManager::Instance()->setCamera(CameraManager::PLAYER, hDebugCamera);
		//SceneNode *pCamSN = debugCamera->getCamSceneNode();

		}
	m_framesWithNoData = 0;
	m_framesWithData = 0;
#endif // #ifdef _XBOX
}

void Flyer::addDefaultComponents()
{
	Component::addDefaultComponents();

	// custom methods of this component
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, Flyer::do_UPDATE);
}

void Flyer::do_UPDATE(PE::Events::Event *pEvt)
{
#ifdef _XBOX
	PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
	float frameTime = pRealEvt->m_frameTime;

// NUI testing
	if( WAIT_OBJECT_0 != WaitForSingleObject( m_hFrameEndEvent, NUI_FRAME_END_TIMEOUT_DEFAULT ) )
	{
		//return FALSE;
		//todo: output this on screen
		//PEINFO("NUI Frame not ready yet\n");
	}
	else
	{
		/*
		// Get data from the next camera color frame
		HRESULT hrImage = NuiImageStreamGetNextFrame( m_hImage, 0, &m_pImageFrame );
		// Get data from the next camera depth frame
		HRESULT hrDepth = NuiImageStreamGetNextFrame( m_hDepth, 0, &m_pDepthFrame );
		*/
		// Get data from the next skeleton frame
		HRESULT hrSkeleton = NuiSkeletonGetNextFrame( 0, &m_SkeletonFrame );

		/*
		if( SUCCEEDED ( hrImage ) )
		{
			m_pip.SetColorTexture( m_pImageFrame->pFrameTexture );
			NuiImageStreamReleaseFrame( m_hImage, m_pImageFrame );
		}

		if( SUCCEEDED( hrDepth ) )
		{

			m_pip.SetDepthTexture( m_pDepthFrame->pFrameTexture );
			NuiImageStreamReleaseFrame( m_hDepth, m_pDepthFrame );
		}*/

		if ( SUCCEEDED( hrSkeleton ) )
		{
			/*
			m_pip.SetSkeletons( &m_SkeletonFrame );
			ApplyTiltCorrection( &m_SkeletonFrame );

			m_dwCurrentFrameIndex = ( m_dwCurrentFrameIndex + 1 ) & 1;
			*/
		}

		if (++m_framesWithNoData > 30 && m_prevCameraType != CameraManager::CameraType_Count)
		{
			CameraManager::Instance()->selectActiveCamera(m_prevCameraType);
			m_prevCameraType = CameraManager::CameraType_Count;
		}
		
		if (SUCCEEDED ( hrSkeleton ) )
		{
			bool found = false;
			// Update if skeleton was tracked
			for (UINT uCurrentSkeleton = 0; uCurrentSkeleton < NUI_SKELETON_COUNT; uCurrentSkeleton++)
			{
				 

				if ( m_SkeletonFrame.SkeletonData[uCurrentSkeleton].eTrackingState == NUI_SKELETON_TRACKED )
				{
					
					found = true;
										
					m_JointFilter.Update( &m_SkeletonFrame.SkeletonData[uCurrentSkeleton] );
				
					// exmaple of fetching Positions only
				
					XMVECTOR vHead = m_SkeletonFrame.SkeletonData[uCurrentSkeleton].SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
				 	
					//Uncomment to enable printout
					
					//PEINFO(Position of head: "%f %f %f\n", vHead.x, vHead.y, vHead.z);
					
					// in case you need orientation of nodes, use the code below
					{
						m_pNuiJointConverterConstrained->ConvertNuiToAvatarSkeleton(&m_SkeletonFrame.SkeletonData[uCurrentSkeleton], m_AvatarJointPoseNuiMapperConstrained);
						m_pNuiJointConverterConstrained->GetNuiSkeletonData(&m_SkeletonFrame.SkeletonData[uCurrentSkeleton]);
						
						Vector3 leftHip, rightHip, hipCenter, leftHand, rightHand, head, leftLeg, rightLeg;
						leftHand = m_lastLeftPos;
						rightHand = m_lastRightPos;
						Matrix4x4 mainBase;

						for (int i = 0; i < XAVATAR_MAX_SKELETON_JOINTS; i++)
						{
							XMVECTOR rot = m_AvatarJointPoseNuiMapperConstrained[i].Rotation;
							XMVECTOR pos = m_pNuiJointConverterConstrained->GetAvatarJointWorldPosition(i);//m_AvatarJointPoseNuiMapperConstrained[i].Position;
							if (pos.z != 0)
								pos.z += 3.0f;
							Quaternion q;
							q.m_x = rot.x;
							q.m_y = rot.y;
							q.m_z = rot.z;
							q.m_w = rot.w;
							Matrix4x4 base(q);
							
							base.setPos(Vector3(pos.x *2.0f, pos.y *2.0f, pos.z * 2.0f));
							
							m_sceneNodes[i]->m_base = base;

							if (m_pNuiJointConverterConstrained->MapAvatarJointToIntIndex(i) == 0 )
							{
								mainBase = base;
							}
							else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_HEAD )
							{
								head = base.getPos();
							}
							else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_KNEE_LEFT )
							{
								leftLeg = base.getPos();
							}
							else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_KNEE_RIGHT )
							{
								rightLeg = base.getPos();
							}else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_HIP_CENTER )
							{
								hipCenter = base.getPos();
							} else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_HIP_LEFT )
							{
								leftHip = base.getPos();
							} else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_HIP_RIGHT )
							{
								rightHip = base.getPos();
							}
							else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_ELBOW_RIGHT )
							{
								rightHand = base.getPos();
							}
							else if (m_pNuiJointConverterConstrained->MapAvatarJointToNUI_POSITION_INDEX(i) == NUI_SKELETON_POSITION_ELBOW_LEFT )
							{
								leftHand = base.getPos();
							}
						}

						static const float HORIZONTAL_SPEED_FACTOR = 0.5f / 1.0f;

						static const float FALL_SPEED_INCREASE = 30.0f / 100.0f;
						static const float ASCEND_SPEED_INCREASE = 8.0f / 100.0f;
						static const float FALL_MAX_SPEED = 10.0f / 100.0f;
						static const float ASCEND_MAX_SPEED = 50.0f / 100.0f;
						static const float TURN_SPEED = .5f;

						Vector3 dif;
						if ((rightHand - m_lastRightPos).m_y < 0)
							dif += ( rightHand - m_lastRightPos );

						if ((leftHand - m_lastLeftPos).m_y < 0)
							dif += ( leftHand - m_lastLeftPos );
						dif = -dif;
						if (dif.m_y > 0.10f)
						{
							m_speed.m_y += ASCEND_SPEED_INCREASE * frameTime * dif.m_y * 100.0f;
							if (m_speed.m_y > ASCEND_MAX_SPEED)
								m_speed.m_y = ASCEND_MAX_SPEED;
						}
						else
						{
							
							// decrease speed
							m_speed.m_y -= FALL_SPEED_INCREASE * frameTime;
							if (m_speed.m_y < -FALL_MAX_SPEED)
								m_speed.m_y = -FALL_MAX_SPEED;
								
						}

						
						Vector3 pos = m_pNuiSN->m_base.getPos();
						m_speed.m_x = m_pNuiSN->m_base.getN().m_x;
						m_speed.m_z = m_pNuiSN->m_base.getN().m_z;
						if (pos.m_y > 1.0f)
						{
							m_speed.m_x *= HORIZONTAL_SPEED_FACTOR * (pos.m_y-1.0f) * frameTime;
							m_speed.m_z *= HORIZONTAL_SPEED_FACTOR * (pos.m_y -1.0f) * frameTime;
						}
						else
						{
							m_speed.m_x = 0;
							m_speed.m_z = 0;
						}
						pos += m_speed;
						
						if (pos.m_y < 1.0f)
							pos.m_y = 1.0f;

						if (pos.m_y > 25.0f)
							pos.m_y = 25.0f;

						m_pNuiSN->m_base.setPos(pos);

						if (hipCenter.m_y - leftLeg.m_y < .9f)
						{
							m_pNuiSN->m_base.turnRight(TURN_SPEED * frameTime);
						}
						else if (hipCenter.m_y - rightLeg.m_y < .9f)
						{
							m_pNuiSN->m_base.turnLeft(TURN_SPEED * frameTime);
						}

						m_pCamSN->m_base = mainBase;
						m_pCamSN->m_base.setPos(hipCenter + Vector3(0, 1.0f, -5.5f));
						m_pCamSN->m_base.turnLeft(3.1415f);
						m_pCamSN->m_base.turnUp(3.1415f / 8.0f);

						SceneNode *pCamSN = CameraManager::Instance()->getCamera(CameraManager::PLAYER)->getCamSceneNode();

						pCamSN->m_base = mainBase;
						pCamSN->m_base.setPos(hipCenter + Vector3(0, 1.0f, -4.5f));
						//pCamSN->m_base.turnLeft(3.1415f);
						pCamSN->m_base.turnDown(3.1415f / 8.0f);

						m_lastRightPos = rightHand;
						m_lastLeftPos = leftHand;

						m_framesWithNoData = 0;
						if (m_prevCameraType == CameraManager::CameraType_Count)
							m_prevCameraType = CameraManager::Instance()->getActiveCameraType(); // code will know to reset it back to prev cam

						CameraManager::Instance()->selectActiveCamera(CameraManager::PLAYER);
					}
					//UpdateNUI( &m_CameraManager.GetSkeleton()->SkeletonData[uCurrentSkeleton] );
					break;
				}
			}
			if (found)
				m_framesWithData++;
			else
				m_framesWithData = 0;
		}
		else
			m_framesWithData = 0;
	}
#endif
}

}; // namespace Components
}; // namespace CharacterControl

//
//  SampleAppViewController.h
//  SampleApp
//
//  Created by artem on 9/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#if 0

#ifndef __ARKANE_GAME_H__
#define __ARKANE_GAME_H__

#define PYENGINE_2_0_MULTI_THREADED

#if APIABSTRACTION_IOS

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>


@interface Game_objc : UIViewController
{
    BOOL animating;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;
	
	int m_frameIndex;
}


@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void)initEngine;
- (void)initGame;
- (void)runSingleFrame;

- (void)unpauseGame;
- (void)pauseGame;

@end

#endif

#include "APIAbstraction/GPUScreen/GPUScreen.h"

#include "PrimeEngineIncludes.h"

namespace PE {
#if APIABSTRACTION_PS3 || APIABSTRACTION_IOS
	extern pthread_mutex_t g_drawThreadLock;
	extern pthread_t g_drawThread;
#else
	extern HANDLE g_drawThreadLock;
	extern HANDLE g_drawThread;
#endif

    namespace Components {
        struct ClientGame;

        struct ClientGlobalGameCallbacks
        {
			typedef Handle (*StaticGameConstruct)(PE::GameContext &context, PE::MemoryArena arena);
			typedef int (*StaticInitEngine)(void);
			static StaticGameConstruct s_constructFunction;

			static StaticInitEngine s_initEngineFunction;
            static Handle s_gameHandle;
            
            static ClientGame *getGameInstance() { return s_gameHandle.getObject<ClientGame>(); }
            
            static Handle InstanceHandle() { return s_gameHandle; }
            
            static void SetGameInstanceHandle(const Handle &handle)
            {
                // Singleton
                s_gameHandle = handle;
            }
            
            static void Construct()
            {
                if (s_constructFunction)
                {
                    s_gameHandle = s_constructFunction();
                }
            }
			static int InitEngine()
			{
				if (s_initEngineFunction)
				{
					return s_initEngineFunction();
				}
				return 0;
			}
        };
        
        
        struct ClientGame : public Component
        {
			struct EngineInitParams
			{
				static const int MAX_ARGS = 32;
				Vector2i m_windowRes;
				const char * m_windowCaption;

				#if APIABSTRACTION_D3D9 || APIABSTRACTION_D3D11 || APIABSTRACTION_GLPC
					HINSTANCE hInstance, hPrevInstance;
				#endif
				const char *lpCmdLine;
				int showCmd;

				const char * args[MAX_ARGS];
				int argc;

				EngineInitParams()
					: m_windowRes(320, 240)
					, m_windowCaption("")
					, lpCmdLine("")
					, showCmd(0)
					#if APIABSTRACTION_D3D9 || APIABSTRACTION_D3D11
						, hInstance(0) , hPrevInstance(0)
					#endif
				{}
				static EngineInitParams s_params;
            };
            
                        // Singleton ------------------------------------------------------------------
            static Handle ConstructCallback(MemoryArena arena)
            {
                Handle handle("GAME", sizeof(ClientGame));
                /*Game *pGame = */ new(handle) ClientGame(context, arena, handle);
                return handle;
            }

			static int InitEngineCallback()
			{
				return PE::Components::ClientGame::initEngine(PE::Components::ClientGame::EngineInitParams::s_params);
			}
            
            ClientGame(Handle hMyself)
            : Component(hMyself)
            #if APIABSTRACTION_IOS
                , m_pGame_objc(NULL)
            #endif
            {
                suspended = true;
                runPhysXSim = true;
                m_hTimer = Handle("TIMER", sizeof(Timer));
                
                m_frameTime = 0;
                m_gameTime = 0;
            }
            

            static int initEngine(EngineInitParams &engineParams);

            virtual int initGame();
            virtual int runGame();
            virtual int runGameFrame();
            static void runGameFrameStatic();
            static void dummyIdleFunction();
            
            #if APIABSTRACTION_IOS
                void setGame_objc(Game_objc *p){m_pGame_objc = p;}
            #endif
            
            bool suspended;
            bool runPhysXSim;
            Handle m_hTimer;
            
            Handle hDefaultGameControls;
            
            PrimitiveTypes::Float32 m_frameTime;
			PrimitiveTypes::Float32 m_gameThreadPreDrawFrameTime;
			PrimitiveTypes::Float32 m_gameThreadDrawWaitFrameTime;
			PrimitiveTypes::Float32 m_gameThreadDrawFrameTime;
			PrimitiveTypes::Float32 m_gameThreadPostDrawFrameTime;

            PrimitiveTypes::Float32 m_gameTime;
            PrimitiveTypes::Bool m_runGame;
            
            
            #if APIABSTRACTION_IOS
                Game_objc *m_pGame_objc;
            #endif
            
        };
                
    }; // namespace Components
}; // namespace PE

#endif
#endif
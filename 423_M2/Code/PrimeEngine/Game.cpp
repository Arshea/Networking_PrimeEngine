#if 0

//
//  Game.m
//  SampleApp
//
//  Created by artem on 9/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#if APIABSTRACTION_IOS
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <CoreData/CoreData.h>
#import "GameIOSInput.h"
#endif

#include "PrimeEngineIncludes.h"
#include "RenderJob.h"

#if APIABSTRACTION_IOS
// Attribute index.
enum {
    ATTRIB_VERTEX,
    ATTRIB_COLOR,
    NUM_ATTRIBUTES
};

@interface Game_objc()
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;
- (BOOL)loadShaders;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;
@end

@implementation Game_objc

@synthesize animating, context, displayLink;

- (void)awakeFromNib
{
	printf("Game::awakeFromNib Entry.. (the game was created through Interface Builder)\n");
	
    
    PE::OGL_GPUScreen::setOGL_GPUScreen_objc((OGL_GPUScreen_objc *)self.view); // static link so that engine knows of the Screen obj from nib
    
	// initialise engine 
	PE::Components::ClientGame::EngineInitParams engineParams;
	PE::Components::ClientGame::initEngine(engineParams);
    
    //if ([context API] == kEAGLRenderingAPIOpenGLES2)
        [self loadShaders];
    
    animating = FALSE;
    animationFrameInterval = 1;
    self.displayLink = nil;
	[self initGame];
    [self.view setMultipleTouchEnabled:YES];
    
    
    // construct
    PE::Components::ClientGlobalGameCallbacks::Construct();
    
    // initialise game
    PE::Components::ClientGlobalGameCallbacks::getGameInstance()->initGame();
    
    PE::Components::ClientGlobalGameCallbacks::getGameInstance()->setGame_objc(self);
	// the game will run through os events handled in OGL_GPUScreenAppDelegate
	
}

- (void)dealloc
{
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    [context release];
    
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self unpauseGame];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self pauseGame];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];

    printf("Unintialise Context Here: FIX IT!\n");	
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self pauseGame];
            [self unpauseGame];
        }
    }
}

- (void)unpauseGame
{
	printf("PE: Progress: Game::unpauseGame Entry..\n");
	
    if (!animating)
    {
        CADisplayLink *aDisplayLink = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(runSingleFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;
        
        animating = TRUE;
    }
}

- (void)pauseGame
{
	printf("PE: Progress: Game::pauseGame Entry..\n");
	
    if (animating)
    {
		printf("PE: Progress: pausing game, invalidating displayLink\n");
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;
    }
}

- (void)drawFrame
{
    // Replace the implementation of this method to do your own custom drawing.
    static int count=0;
    
    count++;
    
    printf("Count is now %d", count);
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source)
    {
        NSLog(@"Failed to load vertex shader");
        return FALSE;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        glDeleteShader(*shader);
        return FALSE;
    }
    
    return TRUE;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (void)initEngine
{
	printf("Game::initEngine Entry..\n");
}

- (void)initGame
{
	printf("Game::initGame Entry..\n");
}

- (void)runSingleFrame
{
	//printf("Game_objc::runSingleFrame[%d] Entry..\n", m_frameIndex);

    PE::Components::ClientGlobalGameCallbacks::getGameInstance()->runGameFrame();
    
	m_frameIndex++;
}

- (BOOL)loadShaders
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname])
    {
        NSLog(@"Failed to compile vertex shader");
        return FALSE;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname])
    {
        NSLog(@"Failed to compile fragment shader");
        return FALSE;
    }
    
    // Release vertex and fragment shaders.
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    
    return TRUE;
}

@end
#endif // end IOS Game wrapper implementation

#include "game.h"
#ifdef _XBOX
#include <xbdm.h>
#endif

#if APIABSTRACTION_PS3
#include <cell/sysmodule.h>
#include <sys/process.h>
#include <sys/spu_initialize.h>
#include <sys/paths.h>
#endif

// Static member variables
PE::Handle PE::Components::ClientGlobalGameCallbacks::s_gameHandle;


PE::Components::ClientGame::EngineInitParams PE::Components::ClientGame::EngineInitParams::s_params;
PE::Components::ClientGlobalGameCallbacks::StaticGameConstruct PE::Components::ClientGlobalGameCallbacks::s_constructFunction = &PE::Components::ClientGame::ConstructCallback;
PE::Components::ClientGlobalGameCallbacks::StaticInitEngine PE::Components::ClientGlobalGameCallbacks::s_initEngineFunction = &PE::Components::ClientGame::InitEngineCallback;


namespace PE {

using namespace Events;


	// gloabl vars
#if APIABSTRACTION_PS3 || APIABSTRACTION_IOS
    pthread_mutex_t g_drawThreadLock;
    pthread_t g_drawThread;
#else
    HANDLE g_drawThreadLock, g_drawThread;
#endif

namespace Components {
int ClientGame::initEngine(EngineInitParams &engineParams)
{
    OutputDebugStringA("PYENGINE LAUNCHING\n");
    
#ifdef _XBOX
    {
        HRESULT hr = DmMapDevkitDrive();
        if (SUCCEEDED(hr))
        {
            OutputDebugStringA("PE: PROGRESS: Mounted DEVKIT:\\\n");
        }
        else
        {
            OutputDebugStringA("PE: ERROR: PROGRESS: Failed to Mount DEVKIT:\\\n");
        }
        
        XNetStartupParams xnsp;
        memset(&xnsp, 0, sizeof(xnsp));
        xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
        xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
        INT err = XNetStartup(&xnsp);
        if (err == 0)
        {
            OutputDebugStringA("PE: PROGRESS: XNetStartupParams Succeeded\n");
        }
        else
        {
            OutputDebugStringA("PE: ERROR: XNetStartupParams Failed\n");
        }
    }
#endif
#if defined(SN_TARGET_PS3)
    
    pthread_attr_t attr;
    size_t stackSize = 0;
    size_t newStackSize = 512*1024;
    
    int rc = pthread_attr_init(&attr);
    if(rc != 0) { 
        return -1;
    }
    
    pthread_attr_getstacksize(&attr, &stackSize);
    
    printf("PE: PROGRESS: Checking stack size = %d\nChanging to %d ..", (int)(stackSize), (int)(newStackSize));
    
    rc = pthread_attr_setstacksize(&attr, newStackSize);
    if(rc != 0) {
        printf("PE: ERROR: pthread_attr_setstacksize failed errcode = %d", rc);
        return -1;
    }
    
    pthread_attr_getstacksize(&attr, &stackSize);
    printf("PE: PROGRESS: Checking stack size = %d\n", (int)(stackSize));
    
    
    // Initialize 6 SPUs but reserve 1 SPU as a raw SPU for PSGL
    sys_spu_initialize(6, 1);	
    
    int ret;
    
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
    if (ret < 0) {
        printf("cellSysmoduleLoadModule() failed (%d)\n", ret);
        exit(1);
    }
    
#endif
    
    srand((unsigned int)(time(NULL)));
    
    MemoryManager::Construct();

    OutputDebugStringA("PE: PROGRESS: MemoryManager Constructed\n");
    
#	if APIABSTRACTION_D3D9 || APIABSTRACTION_D3D11
    MainFunctionArgs::Construct(engineParams.lpCmdLine, engineParams.hInstance);
#	else
    MainFunctionArgs::Construct(engineParams.lpCmdLine);
#	endif

    LuaEnvironment::ConstructStart();
    OutputDebugStringA("PE: PROGRESS: LuaEnvironment Constructed (No handlers added yet)\n");
    
    PE::Register();
    OutputDebugStringA("PE: PROGRESS: Registered all PE Components and Events\n");
                            
    LuaEnvironment::ConstructEnd();
    OutputDebugStringA("PE: PROGRESS: LuaEnvironment Constructed (Added event handlers now that the events are registered)\n");       
#ifdef _XBOX
    XNADDR          g_xnaddr;                      // our own XNADDR
    // Get our own XNADDR
    DWORD dwRet;
    do
    {
        dwRet = XNetGetTitleXnAddr( &g_xnaddr );
    } while( dwRet == XNET_GET_XNADDR_PENDING );
    
    OutputDebugStringA("Machine IP Address is :");
    
    printf( " XNADDR: %02X:%02X:%02X:%02X:%02X:%02X\n",
           g_xnaddr.abEnet[0], g_xnaddr.abEnet[1], g_xnaddr.abEnet[2],
           g_xnaddr.abEnet[3], g_xnaddr.abEnet[4], g_xnaddr.abEnet[5] );

#endif
    
    GPUScreen::Construct(engineParams.m_windowRes.m_xi, engineParams.m_windowRes.m_yi, engineParams.m_windowCaption);
    
    OutputDebugStringA("PE: PROGRESS: GPUScreen Constructed\n");
    

    GameObjectManager::Construct();
    
    OutputDebugStringA("PE: PROGRESS: GameObjectManager Constructed\n");
    

	NetworkManager::Construct();

    Input::Construct();
    
    OutputDebugStringA("PE: PROGRESS: Input Constructed\n");
    
    // Input do_UPDATE() will put events on input queue
    GameObjectManager::Instance()->addComponent(Input::s_hMyself);
    
    
    //Construct Single Event Handlers
    Log::Construct();
    
	Profiling::Profiler::Construct();

    SingleHandler_DRAW::Construct();
    PESSEH_CHANGE_TO_DEBUG_SHADER::Construct();
    PESSEH_POP_SHADERS::Construct();
    PESSEH_DRAW_Z_ONLY::Construct();
                
    Events::EventQueueManager::Construct();
    
#ifdef PYENGINE_USE_PHYSX
    Physics::Initialize();
    //Physics::LoadScene(); //Loading city/floor generated phys rep. At this point NVIDIA has not provided an exporter for MAYA
    Physics::LoadDefaults();
#endif
    EffectManager::Construct();
    
    OutputDebugStringA("PE: PROGRESS: EffectManager Constructed\n");
    
	VertexBufferGPUManager::Construct();

    // This will load default effects (shaders)
    EffectManager::Instance()->loadDefaultEffects();
    
    GPUTextureManager::Construct();
    AnimationSetCPUManager::Construct();
    
    PositionBufferCPUManager::Construct();
    NormalBufferCPUManager::Construct();
    TexCoordBufferCPUManager::Construct();
    
    DrawList::Construct();
    
    RootSceneNode::Construct();
	DebugRenderer::Construct();
	CameraManager::Construct();
    
    // initialize timer functionality
    Timer::Initialize();
                
    return 1;
}
        
                
                
int ClientGame::initGame()
{
    return 1;
}
void ClientGame::runGameFrameStatic()
{
    ClientGlobalGameCallbacks::getGameInstance()->runGameFrame();
}

void ClientGame::dummyIdleFunction()
{
	#if APIABSTRACTION_OGL && APIABSTRACTION_GLPC
		glutPostRedisplay();
	#endif
}

int ClientGame::runGame()
{
    // create a timer for frame time tracking
    /*Timer *pT = */new(m_hTimer) Timer();
    
    // Game Loop ---------------------------------------------------------------
    m_runGame = true;
    
#ifdef PYENGINE_2_0_MULTI_THREADED
    // todo: implement with pthreads
    
    // drawing thread
    g_drawThreadLock = CreateMutex(0, false, 0);
    
    unsigned int threadId;
    g_drawThread =(HANDLE)_beginthreadex(NULL,
                                         0,
                                         drawThreadFunctionJob,
                                         0,
                                         CREATE_SUSPENDED,
                                         &threadId);
#endif
    
    
#if APIABSTRACTION_OGL && APIABSTRACTION_GLPC
    glutDisplayFunc(ClientGame::runGameFrameStatic);
    glutIdleFunc(ClientGame::dummyIdleFunction);
    glutMainLoop();
#else
    while(m_runGame)
    {
        runGameFrame();
    } // while (runGame) -- game loop
#endif
    return 0;
}
        
int ClientGame::runGameFrame()
{
    // Frame Start ---------------------------------------------------------
    float gameThreadPreDrawFrameTime = 0;
	float gameThreadDrawFrameTime = 0;
	float gameThreadDrawWaitFrameTime = 0;
	// cache root scene node pointer
    RootSceneNode *proot = RootSceneNode::Instance();
    
    //Start Logging events for this frame if requested
    if(Log::Instance()->m_activateOnNextFrame)
    {
        Log::Instance()->m_isActivated = true;
        Log::Instance()->m_activateOnNextFrame = false;
    }
    
    // CLOSED_WINDOW event will be pushed into global event queue if user closes window
    // after this call
#		if APIABSTRACTION_D3D9
    D3D9_GPUScreen::Instance()->processOSEventsIntoGlobalEventQueue();
#		elif APIABSTRACTION_D3D11
    D3D11_GPUScreen::Instance()->processOSEventsIntoGlobalEventQueue();
#		endif
    
    //Create Physics Events
    {
        Handle hphysStartEvent("EVENT", sizeof(Event_PHYSICS_START));
        Event_PHYSICS_START *physStartEvent = new(hphysStartEvent) Event_PHYSICS_START ;
        
        physStartEvent->m_frameTime = m_frameTime;
        Events::EventQueueManager::Instance()->add(hphysStartEvent, Events::QT_GENERAL);
    }
    // Create UPDATE event
    {
        Handle hupdateEvent("EVENT", sizeof(Event_UPDATE));
        Event_UPDATE *updateEvent = new(hupdateEvent) Event_UPDATE ;
        
        updateEvent->m_frameTime = m_frameTime;
        
        //Push UPDATE event onto general queue
        Events::EventQueueManager::Instance()->add(hupdateEvent, Events::QT_GENERAL);
    }
    // Create SCENE_GRAPH_UPDATE event 
    {
        Handle hsgUpdateEvent("EVENT", sizeof(Event_SCENE_GRAPH_UPDATE));
        Event_SCENE_GRAPH_UPDATE *sgUpdateEvent = new(hsgUpdateEvent) Event_SCENE_GRAPH_UPDATE ;
        
        sgUpdateEvent->m_frameTime = m_frameTime;
        //Push event into general queue
        Events::EventQueueManager::Instance()->add(hsgUpdateEvent, Events::QT_GENERAL);
    }
    
    //Assign camera
    Handle hCam = CameraManager::Instance()->getActiveCameraHandle();
    CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
    
    // Push Event_CALCULATE_TRANSFORMATIONS
    {
        Handle hctevt("EVENT", sizeof(Event_CALCULATE_TRANSFORMATIONS));
        /*Event_CALCULATE_TRANSFORMATIONS *ctevt = */ new(hctevt) Event_CALCULATE_TRANSFORMATIONS ;
        
        Events::EventQueueManager::Instance()->add(hctevt, Events::QT_GENERAL);
    }
    
    // Process general events (Draw, Update, Calculate transformations...)
    Handle gqh = Events::EventQueueManager::Instance()->getEventQueueHandle("general");
    while (!gqh.getObject<Events::EventQueue>()->empty())
    {
        Events::Event *pGeneralEvt = gqh.getObject<Events::EventQueue>()->getFront();
        // this code is in process of conversion to new event style
        // first use new method then old (switch)
        if (Event_UPDATE::GetClassId() == pGeneralEvt->getClassId())
        {
            // UPDATE
            // Update game objects
            GameObjectManager::Instance()->handleEvent(pGeneralEvt);
        }
        else if (Event_CALCULATE_TRANSFORMATIONS::GetClassId() == pGeneralEvt->getClassId())
        {
            
            // for all scene objects to calculate their absolute (world) transformations
            // for skins to calculate their matrix palettes
            proot->handleEvent(pGeneralEvt);
            
            //SkyVolume::Instance()->handleEvent(pGeneralEvt);
            
            // Generate Drawing Events
            {
                Handle hdrawZOnlyEvt("EVENT", sizeof(Event_GATHER_DRAWCALLS_Z_ONLY));
                Event_GATHER_DRAWCALLS_Z_ONLY *drawZOnlyEvt = new(hdrawZOnlyEvt) Event_GATHER_DRAWCALLS_Z_ONLY ;
                
                drawZOnlyEvt->m_pZOnlyDrawListOverride = 0;

				RootSceneNode *pRoot = RootSceneNode::Instance();

				if (pRoot->m_lights.m_size)
				{
					PrimitiveTypes::Bool foundShadower = false;
					for(PrimitiveTypes::UInt32 i=0; i<(pRoot->m_lights.m_size); i++){
						Light *pLight = pRoot->m_lights[i].getObject<Light>();
						if(pLight->castsShadow()){

						#if APIABSTRACTION_OGL
							drawZOnlyEvt->m_projectionViewTransform = (pLight->m_worldToViewTransform * pLight->m_viewToProjectedTransform);
						#else
							drawZOnlyEvt->m_projectionViewTransform = (pLight->m_viewToProjectedTransform * pLight->m_worldToViewTransform);
						#endif
							drawZOnlyEvt->m_eyePos = pLight->m_base.getPos();
							foundShadower=true;
							break;
						}
						if(!foundShadower){
						#if APIABSTRACTION_OGL
							drawZOnlyEvt->m_projectionViewTransform = pcam->m_worldToViewTransform * pcam->m_viewToProjectedTransform;
						#else
							drawZOnlyEvt->m_projectionViewTransform = pcam->m_viewToProjectedTransform * pcam->m_worldToViewTransform;
						#endif
							drawZOnlyEvt->m_eyePos = pcam->m_worldTransform.getPos();
						}
					}
				}
				else
				{
					#if APIABSTRACTION_OGL
						drawZOnlyEvt->m_projectionViewTransform = pcam->m_worldToViewTransform * pcam->m_viewToProjectedTransform;
					#else
						drawZOnlyEvt->m_projectionViewTransform = pcam->m_viewToProjectedTransform * pcam->m_worldToViewTransform;
					#endif
					drawZOnlyEvt->m_eyePos = pcam->m_worldTransform.getPos();
				}
                drawZOnlyEvt->m_parentWorldTransform.loadIdentity();
                Events::EventQueueManager::Instance()->add(hdrawZOnlyEvt);
            }
            
            // After the transformations are done. We can put a DRAW event in the queue
            // Push DRAW event into message queue because camera has updated transformations
            {
                Handle hdrawEvt("EVENT", sizeof(Event_GATHER_DRAWCALLS));
                Event_GATHER_DRAWCALLS *drawEvt = new(hdrawEvt) Event_GATHER_DRAWCALLS ;
                
                drawEvt->m_frameTime = m_frameTime;
                drawEvt->m_gameTime = m_gameTime;
                
                drawEvt->m_drawOrder = EffectDrawOrder::First;
                
                //Camera *pcam = hcam.getObject<Camera>();
#if APIABSTRACTION_OGL
                drawEvt->m_projectionViewTransform = pcam->m_viewToProjectedTransform * pcam->m_worldToViewTransform ;
#else
                drawEvt->m_projectionViewTransform = pcam->m_viewToProjectedTransform * pcam->m_worldToViewTransform;
#endif
                drawEvt->m_eyePos = pcam->m_worldTransform.getPos();
				drawEvt->m_projectionTransform = pcam->m_viewToProjectedTransform;
				drawEvt->m_eyeDir = pcam->m_worldTransform.getN();
                drawEvt->m_parentWorldTransform.loadIdentity();
                drawEvt->m_viewInvTransform = pcam->m_worldToViewTransform.inverse();
                
				//Commented out by Mac because I'm pretty sure this does nothing but am afraid to delete it...
				/*static bool setCameraAsLightSource = false;
				RootSceneNode *pRoot = RootSceneNode::Instance();
				if (setCameraAsLightSource && pRoot->m_lights.m_size)
				{
					Light *pLight = pRoot->m_lights[0].getObject<Light>();

					#if APIABSTRACTION_OGL
						drawEvt->m_projectionViewTransform = (pLight->m_worldToViewTransform * pLight->m_viewToProjectedTransform);
					#else
						drawEvt->m_projectionViewTransform = (pLight->m_viewToProjectedTransform * pLight->m_worldToViewTransform);
					#endif
					drawEvt->m_eyePos = pLight->m_base.getPos();
				}*/ 
				
                Events::EventQueueManager::Instance()->add(hdrawEvt);
            }
            
            {
                Handle hphysicsEndEvt("EVENT", sizeof(Event_PHYSICS_END));
                /*Event_PHYSICS_END *physicsEndEvt = */ new(hphysicsEndEvt) Event_PHYSICS_END ;
                
                Events::EventQueueManager::Instance()->add(hphysicsEndEvt);
            }
            
        }
        
        else if (Event_GATHER_DRAWCALLS::GetClassId() == pGeneralEvt->getClassId()
			|| Event_GATHER_DRAWCALLS_Z_ONLY::GetClassId() == pGeneralEvt->getClassId())
        {
			bool zOnly = Event_GATHER_DRAWCALLS_Z_ONLY::GetClassId() == pGeneralEvt->getClassId();

            Event_GATHER_DRAWCALLS *pDrawEvent = NULL;
			Event_GATHER_DRAWCALLS_Z_ONLY *pDrawZOnlyEvent = NULL;
			if (zOnly)
			{
				pDrawZOnlyEvent = (Event_GATHER_DRAWCALLS_Z_ONLY *)(pGeneralEvt);
				DrawList::ZOnlyInstance()->reset();

			}
			else
			{
				pDrawEvent = (Event_GATHER_DRAWCALLS *)(pGeneralEvt);
				DrawList::Instance()->reset();
			}

			if (pDrawEvent)
				EffectManager::Instance()->m_currentViewProjMatrix = pDrawEvent->m_projectionViewTransform;

            
			// Draw 1st order
			proot->handleEvent(pGeneralEvt);

			// for non z only we do several draw order passes
			if (pDrawEvent)
			{
				// Manual draw pass
				pDrawEvent->m_drawOrder = EffectDrawOrder::Manual;
			
				// Draw SkyVolume
				//SkyVolume::Instance()->handleEvent(pGeneralEvt);

				EffectManager::Instance()->m_doMotionBlur = true;

				// Draw Last order
				pDrawEvent->m_drawOrder = EffectDrawOrder::Last;
			
				proot->handleEvent(pGeneralEvt);
			}

			// this code will make sure draw thread know we are done
			// only do it for DRAW event since is last draw event
			// and in case we don't do multithreading, we just execute the draw thread function

			if (!zOnly)
			{
				gameThreadPreDrawFrameTime = m_hTimer.getObject<Timer>()->TickAndGetTimeDeltaInSeconds();

				static bool s_RenderOnGameThread = false;

				#ifdef PYENGINE_2_0_MULTI_THREADED
				if (!s_RenderOnGameThread)
					WaitForSingleObject(g_drawThreadLock, 100000); // wait till previous draw is finished
					//OutputDebugStringA("Game thread got g_drawThreadLock\n");
				
					gameThreadDrawWaitFrameTime = m_hTimer.getObject<Timer>()->TickAndGetTimeDeltaInSeconds();
					
					// finalize results of gpu profiling. we want to do it in this thread to avoid race condition
					// since we are accessing debug renderer. 
					Profiling::Profiler::Instance()->finalize(Profiling::Group_DrawCalls);
					Profiling::Profiler::Instance()->finalize(Profiling::Group_DrawThread);

					// send this event to objects so that they have ability to work with graphics resources
					// since this thread has ownership of dx thread
					Event_PRE_RENDER_needsRC preRenderEvt;
					GameObjectManager::Instance()->handleEvent(&preRenderEvt);
					proot->handleEvent(&preRenderEvt);
					
					{
						float fps = (1.0f/m_frameTime);
						sprintf(PEString::s_buf, "%.2f FPS", fps);
						Event_SET_MESH_TEXT fpsEvt (PEString::s_buf, true, false, false, false, 0, Vector3(.5f, .85f, 0), 0.5f);
						DebugRenderer::Instance()->handleEvent(&fpsEvt);
					}
					//gameplay timer
					{
						sprintf(PEString::s_buf, "Game thread pre-draw: %f + render wait: %f + render: %f = %f sec\n", m_gameThreadPreDrawFrameTime+m_gameThreadPostDrawFrameTime, m_gameThreadDrawWaitFrameTime, m_gameThreadDrawFrameTime, m_frameTime);
						Event_SET_MESH_TEXT fpsEvt (PEString::s_buf, true, false, false, false, 0, Vector3(-1.0f, .8f, 0), 0.45f);
						DebugRenderer::Instance()->handleEvent(&fpsEvt);
					}


					DrawList::swap();
					//OutputDebugStringA("Game thread releasing g_drawThreadLock\n");
					ReleaseMutex(g_drawThreadLock); // allow to draw
					ResumeThread(g_drawThread);
					if (s_RenderOnGameThread) // wait here until render is finished
						WaitForSingleObject(g_drawThreadLock, 100000); // wait till previous draw is finished
				#else
					DrawList::swap();
					drawThreadFunction(0);
				#endif

				gameThreadDrawFrameTime = m_hTimer.getObject<Timer>()->TickAndGetTimeDeltaInSeconds();

			}
        }
        else if (Event_FLY_CAMERA::GetClassId() == pGeneralEvt->getClassId())
        {
            Event_FLY_CAMERA *pRealEvent = (Event_FLY_CAMERA *)(pGeneralEvt);
            pcam->m_base.moveForward(pRealEvent->m_relativeMove.getZ());
            pcam->m_base.moveRight(pRealEvent->m_relativeMove.getX());
            pcam->m_base.moveUp(pRealEvent->m_relativeMove.getY());
        }
        else if (Event_ROTATE_CAMERA::GetClassId() == pGeneralEvt->getClassId())
        {
            Event_ROTATE_CAMERA *pRealEvent = (Event_ROTATE_CAMERA *)(pGeneralEvt);
            pcam->m_base.turnUp(pRealEvent->m_relativeRotate.getY());
            pcam->m_base.turnAboutAxis(-pRealEvent->m_relativeRotate.getX(), RootSceneNode::Instance()->m_worldTransform.getV());
        }
        
        else if (Event_CLOSED_WINDOW::GetClassId() == pGeneralEvt->getClassId())
        {
            m_runGame = false;
        }
        else if (Event_SCENE_GRAPH_UPDATE::GetClassId() == pGeneralEvt->getClassId())
        {
            // this event is meant for scene graph
            proot->handleEvent(pGeneralEvt);
        }
        else if (Event_PHYSICS_END::GetClassId() == pGeneralEvt->getClassId())
        {
#ifdef PYENGINE_USE_PHYSX
            // Try fetching data here.. if not available yet, set some flag, so tht on next PHYSICS_START we dont run simulate
            runPhysXSim = Physics::GetResults();
#endif
        }
        else if (Event_PHYSICS_START::GetClassId() == pGeneralEvt->getClassId())
        {
#ifdef PYENGINE_USE_PHYSX
            Physics::UpdateControllers();
            Physics::UpdateKinematics();
            Physics::UpdateForceFields(frameTime);
            // Update Simulation
            Physics::Update(frameTime);
            // game object's event handlers will synch back to physx actors in this call
            GameObjectManager::Instance()->handleEvent(pGeneralEvt);
#endif
        }
#ifdef PYENGINE_USE_PHYSX
        else if (Event_PHYSICS_COLLISION::GetClassId() == pGeneralEvt->getClassId())
        {
            GameObjectManager::Instance()->handleEvent(pGeneralEvt);
        }
#endif
        else
        {
            // unknown event
            // pass it to both game object manager and scene graph
            
            GameObjectManager::Instance()->handleEvent(pGeneralEvt);
            proot->handleEvent(pGeneralEvt);
            
        } // end of old event style handling
        
        gqh.getObject<Events::EventQueue>()->destroyFront();
    }
    
    // Events are destoryed by destroyFront() but this is called every frame just in case
    gqh.getObject<Events::EventQueue>()->destroy();
    
    // End of frame --------------------------------------------------------
    
    // add memory defragmentation here
    // after this all pointers must be recalculated from handles
    
    //Stop logging events for this frame if was requested to log in this frame
    Log::Instance()->m_isActivated = false;
    
	float gameThreadPostDrawFrameTime = m_hTimer.getObject<Timer>()->TickAndGetTimeDeltaInSeconds();
	m_frameTime = gameThreadPreDrawFrameTime + gameThreadDrawWaitFrameTime + gameThreadDrawFrameTime + gameThreadPostDrawFrameTime;
	m_gameThreadPreDrawFrameTime = gameThreadPreDrawFrameTime;
	m_gameThreadDrawWaitFrameTime = gameThreadDrawWaitFrameTime;
	m_gameThreadDrawFrameTime = gameThreadDrawFrameTime;
	m_gameThreadPostDrawFrameTime = gameThreadPostDrawFrameTime;

	m_gameTime += m_frameTime;
	return 0;

}
}; // namespace Components
}; // namespace PE

#endif


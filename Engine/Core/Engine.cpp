//-------------------------------------------------------------------------------------------------
//"All the best weapons have names"
//This Engine is named: Bread
//Author: Clay Howell
// - Use with game jam
#include "Engine/Core/Engine.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>

#include "Engine/AudioSystem/BAudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/DebugSystem/BDebugSystem.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/InputSystem/BInputSystem.hpp"
#include "Engine/Math/Vector2i.hpp"
#include "Engine/NetworkSystem/BNetworkSystem.hpp"
#include "Engine/NetworkSystem/RCS/RemoteCommandServer.hpp"
#include "Engine/RenderSystem/Camera3D.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/BSpriteGameRenderer.hpp"
#include "Engine/Threads/BJobSystem.hpp"
#include "Engine/UISystem/UISystem.hpp"


//-------------------------------------------------------------------------------------------------
extern Engine * g_EngineSystem = nullptr;


//-------------------------------------------------------------------------------------------------
//Name on top game window
STATIC float const Engine::PERCENT_OF_SCREEN = 0.8f;
STATIC float const Engine::ASPECT_RATIO = 16.f / 9.f;


//-------------------------------------------------------------------------------------------------
Engine::Engine(HINSTANCE applicationInstanceHandle)
	: m_applicationInstanceHandle(applicationInstanceHandle)
	, m_timeLastFrameBegan(0.0)
	, m_targetFPS(60.0)
	, m_isFullscreen(false)
{
	//Randomize Seed
	srand((unsigned int)time(NULL));

	//Give us correct pixels on screen
	SetProcessDPIAware();

	//Create Application Window
	CreateOpenGLWindow(m_applicationInstanceHandle);

	//Start Engine clock
	m_timeLastFrameBegan = Time::GetCurrentTimeSeconds();

	//Create All Engine Systems
	BMemorySystem::Startup();
	BEventSystem::Startup();
	BAudioSystem::Startup();
	BInputSystem::Startup();
	BRenderSystem::Startup();
	BConsoleSystem::Startup();
	BSpriteGameRenderer::Startup();
	UISystem::Startup();
	BDebugSystem::Startup();
	//#TODO: JobSystem incorrectly triggers memory leak warning when there are no leaks
	//BJobSystem::Startup(-2);
	BNetworkSystem::Startup();
	RemoteCommandServer::Startup();
}


//-------------------------------------------------------------------------------------------------
Engine::~Engine()
{
	RemoteCommandServer::Shutdown();
	BNetworkSystem::Shutdown();
	//BJobSystem::Shutdown();
	BDebugSystem::Shutdown();
	BConsoleSystem::Shutdown();
	UISystem::Shutdown();
	BSpriteGameRenderer::Shutdown();
	BRenderSystem::Shutdown();
	BInputSystem::Shutdown();
	BAudioSystem::Shutdown();
	BEventSystem::Shutdown();
	Clock::DestroyClocks();
	BMemorySystem::Shutdown();
}


//-------------------------------------------------------------------------------------------------
void Engine::Update()
{
	//Update Total time and Delta time
	UpdateTime();

	BProfiler::StartSample("UPDATE ENGINE");

	BInputSystem::Update();
	BRenderSystem::Update();
	BMemorySystem::Update();
	BAudioSystem::Update();
	BDebugSystem::Update();
	BConsoleSystem::Update();
	BEventSystem::TriggerEvent(EVENT_ENGINE_UPDATE);

	//Reset Variables
	BProfiler::StopSample();
}


//-------------------------------------------------------------------------------------------------
void Engine::LateUpdate()
{
	BProfiler::StartSample("UPDATE ENGINE LATE");

	UISystem::Update();

	BProfiler::StopSample();
}


//-------------------------------------------------------------------------------------------------
void Engine::UpdateTime()
{
	//#TODO: Find a nicer way to limit FPS
	BProfiler::StartSample("WAIT FOR FRAME");
	double targetFPS = m_targetFPS;
	double timeThisFrameBegan = Time::GetCurrentTimeSeconds();
	double elapsedTime = (timeThisFrameBegan - m_timeLastFrameBegan);
	double totalFrameTime = 1.0 / targetFPS;

	if(g_limitFPS && elapsedTime < totalFrameTime)
	{
		// Make the sleep stop juuust before desired fps
		double const timeOffset = 0.7;
		double waitTime = (totalFrameTime - elapsedTime) * 1000.0 - timeOffset;
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(waitTime));
	}
	BProfiler::StopSample();

	//Track time
	timeThisFrameBegan = Time::GetCurrentTimeSeconds();
	Time::DELTA_SECONDS = static_cast<float>(timeThisFrameBegan - m_timeLastFrameBegan);
	Time::TOTAL_SECONDS += Time::DELTA_SECONDS;
	m_timeLastFrameBegan = timeThisFrameBegan;

	//Update clock system
	for(Clock * clock : Clock::s_baseClocks)
	{
		clock->Update(Time::DELTA_SECONDS);
	}
}


//-------------------------------------------------------------------------------------------------
void Engine::Render() const
{
	BProfiler::StartSample("RENDER ENGINE");

	BEventSystem::TriggerEvent(EVENT_ENGINE_RENDER);
	UISystem::Render();
	BDebugSystem::Render();
	BConsoleSystem::Render();

	//Double Buffer | Also known as FlipAndPresent
	SwapBuffers(g_displayDeviceContext);

	BProfiler::StopSample();
}


//-------------------------------------------------------------------------------------------------
void Engine::CreateOpenGLWindow(HINSTANCE applicationInstanceHandle)
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	//Original
	//const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	//const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	DWORD windowStyleFlags;
	DWORD windowStyleExFlags;

	//Fullscreen
	// 	if ( TheApp::FULLSCREEN )
	// 	{
	// 		windowStyleFlags = WS_POPUP;
	// 		windowStyleExFlags = WS_EX_APPWINDOW | WS_EX_TOPMOST;
	// 	}
	//Windowed
	//	else
	//{
	windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	windowStyleExFlags = WS_EX_APPWINDOW;
	//}

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	CalculateLargestWindow();

	RECT windowRect = {m_offsetXFromWindowsDesktop,
		m_offsetYFromWindowsDesktop,
		m_offsetXFromWindowsDesktop + m_windowPhysicalWidth,
		m_offsetYFromWindowsDesktop + m_windowPhysicalHeight};
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	m_windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(m_windowHandle, SW_SHOW);
	SetForegroundWindow(m_windowHandle);
	SetFocus(m_windowHandle);

	g_displayDeviceContext = GetDC(m_windowHandle);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat(g_displayDeviceContext, &pixelFormatDescriptor);
	SetPixelFormat(g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
	g_openGLRenderingContext = wglCreateContext(g_displayDeviceContext);
	wglMakeCurrent(g_displayDeviceContext, g_openGLRenderingContext);
}


//-------------------------------------------------------------------------------------------------
void Engine::CalculateLargestWindow()
{
	Vector2i desktopDimensions = GetDesktopDimensions();
	float workingSpaceWidth = (float)desktopDimensions.x * PERCENT_OF_SCREEN;
	float workingSpaceHeight = (float)desktopDimensions.y * PERCENT_OF_SCREEN;
	float workingAspect = workingSpaceWidth / workingSpaceHeight;
	float aspect = GetWindowAspectRatio();

	//Width is largest
	if(workingAspect > aspect)
	{
		m_windowPhysicalHeight = (int)workingSpaceHeight;
		m_windowPhysicalWidth = (int)(workingSpaceHeight * aspect);
	}

	//Height is largest
	else
	{
		m_windowPhysicalWidth = (int)workingSpaceWidth;
		m_windowPhysicalHeight = (int)(workingSpaceWidth / aspect);
	}

	m_offsetXFromWindowsDesktop = (int)((desktopDimensions.x - m_windowPhysicalWidth) / 2.f);
	m_offsetYFromWindowsDesktop = (int)((desktopDimensions.y - m_windowPhysicalHeight) / 2.f);
}


//-------------------------------------------------------------------------------------------------
void Engine::SetTargetFPS(double fps)
{
	m_targetFPS = fps;
}


//-------------------------------------------------------------------------------------------------
void Engine::SetWindowHandle(HWND & handle)
{
	m_windowHandle = handle;
}


//-------------------------------------------------------------------------------------------------
void Engine::SetFullscreen(bool isFullscreen)
{
	if(m_isFullscreen != isFullscreen)
	{
		m_isFullscreen = isFullscreen;
	}
}


//-------------------------------------------------------------------------------------------------
HWND Engine::GetWindowHandle() const
{
	return m_windowHandle;
}


//-------------------------------------------------------------------------------------------------
double Engine::GetTargetFPS() const
{
	return m_targetFPS;
}


//-----------------------------------------------------------------------------------------------
// Function By: Squirrel Eiserloh (from StringUtils.cpp)
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;
void Engine::ConsolePrintf(const char* format, ...)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	BConsoleSystem::AddLog(std::string(textLiteral), Color::WHITE);
}


//-----------------------------------------------------------------------------------------------
// Can't put color at the end of the list
void Engine::ConsolePrintf(Color const & color, const char* format, ...)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	BConsoleSystem::AddLog(std::string(textLiteral), color);
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2i Engine::GetWindowDimensions()
{
	if(g_EngineSystem)
	{
		return Vector2i(g_EngineSystem->m_windowPhysicalWidth, g_EngineSystem->m_windowPhysicalHeight);
	}
	else
	{
		return Vector2i(Engine::START_WINDOW_WIDTH, Engine::START_WINDOW_HEIGHT);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2i Engine::GetDesktopDimensions()
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	int horizontal = desktop.right;
	int vertical = desktop.bottom;

	return Vector2i(horizontal, vertical);
}


//-------------------------------------------------------------------------------------------------
STATIC float Engine::GetWindowAspectRatio()
{
	return ASPECT_RATIO;
}


//-------------------------------------------------------------------------------------------------
STATIC Vector3f Engine::ScreenToClipSpace(Vector2f const & screenPosition)
{
	Vector2i windowDimensions = GetWindowDimensions();
	float aspectRatio = (float)windowDimensions.x / (float)windowDimensions.y;
	float xPosition = screenPosition.x / (float)windowDimensions.x;
	xPosition = (xPosition * 2.f - 1.f)*(aspectRatio);
	float yPosition = screenPosition.y / (float)windowDimensions.y;
	yPosition = (yPosition * 2.f - 1.f);
	return Vector3f(xPosition, yPosition, 0.f);
}
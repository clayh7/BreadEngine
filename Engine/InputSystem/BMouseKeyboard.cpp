#include "Engine/InputSystem/BMouseKeyboard.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
//#include "Engine/EventSystem/EventSystem.hpp"
//#include "Engine/InputSystem/BXboxController.hpp"
//#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
STATIC BMouseKeyboard * BMouseKeyboard::s_Instance = nullptr;
STATIC char const * BMouseKeyboard::EVENT_TYPED_CHAR = "EventTypedChar";
STATIC char const * BMouseKeyboard::EVENT_BACKSPACE_PRESSED = "EventBackspacePressed";
STATIC char const * BMouseKeyboard::EVENT_KEY_DOWN = "EventKeyDown";
STATIC char const * BMouseKeyboard::EVENT_KEY_UP = "EventKeyUp";
STATIC char const * BMouseKeyboard::EVENT_MOUSE_WHEEL = "EventMouseWheel";
STATIC char const * BMouseKeyboard::EVENT_MOUSE_DOWN = "EventMouseDown";
STATIC char const * BMouseKeyboard::EVENT_MOUSE_UP = "EventMouseUp";
STATIC char const * BMouseKeyboard::EVENT_FOCUS_GAINED = "EventFocusGained";
STATIC char const * BMouseKeyboard::EVENT_FOCUS_LOST = "EventFocusLost";
STATIC char const * BMouseKeyboard::PARAM_MOUSE_BUTTON = "MouseButton";
STATIC char const * BMouseKeyboard::PARAM_MOUSE_WHEEL = "MouseWheel";
STATIC char const * BMouseKeyboard::PARAM_KEY = "Key";


//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	//Only run if these both exist
	if(!BConsoleSystem::s_System)
	{
		return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
	}

	KeyCode asKey = static_cast<KeyCode>(wParam);

	switch(wmMessageCode)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		g_isQuitting = true;
		return 0;

	case WM_KEYDOWN:
		//if(g_ConsoleSystem->IsOpen())
		//{
		//	g_ConsoleSystem->PressKey(asKey);
		//}
		//else
		{
			BMouseKeyboard::SetKeyPressed(asKey);
		}
		break;

	case WM_KEYUP:
		//if(g_ConsoleSystem->IsOpen())
		//{
		//	g_ConsoleSystem->ReleaseKey(asKey);
		//}
		//else
		{
			BMouseKeyboard::SetKeyReleased(asKey);
		}
		break;

		//This gets called before input is created
	case WM_SETFOCUS:
		BMouseKeyboard::SetFocusGained();
		break;

	case WM_KILLFOCUS:
		BMouseKeyboard::SetFocusLost();
		break;

	case WM_LBUTTONDOWN:
		//if(!g_ConsoleSystem->IsOpen())
		{
			BMouseKeyboard::SetMouseButtonPressed(eMouseButton_LEFT);
		}
		break;

	case WM_LBUTTONUP:
		//if(!g_ConsoleSystem->IsOpen())
		{
			BMouseKeyboard::SetMouseButtonReleased(eMouseButton_LEFT);
		}
		break;
	
	case WM_MBUTTONDOWN:
		BMouseKeyboard::SetMouseButtonPressed(eMouseButton_MIDDLE);
		break;

	case WM_MBUTTONUP:
		BMouseKeyboard::SetMouseButtonReleased(eMouseButton_MIDDLE);
		break;

	case WM_RBUTTONDOWN:
		//if(!g_ConsoleSystem->IsOpen())
		{
			BMouseKeyboard::SetMouseButtonPressed(eMouseButton_RIGHT);
		}
		break;

	case WM_RBUTTONUP:
		//if(!g_ConsoleSystem->IsOpen())
		{
			BMouseKeyboard::SetMouseButtonReleased(eMouseButton_RIGHT);
		}
		break;


	case WM_MOUSEWHEEL:
		//if(g_ConsoleSystem->IsOpen())
		//{
		//	g_ConsoleSystem->MoveMouseWheel((int)GET_WHEEL_DELTA_WPARAM(wParam));
		//}
		//else
		{
			BMouseKeyboard::SetMouseWheelChanged((int)GET_WHEEL_DELTA_WPARAM(wParam));
		}
		break;

	case WM_CHAR:
		BMouseKeyboard::TypedCharacter(asKey);
		break;
	}

	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


//-------------------------------------------------------------------------------------------------
void RunMessagePump()
{
	MSG queuedMessage;
	for(;;)
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if(!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::Update()
{
	if(s_Instance)
	{
		s_Instance->m_currentFrameNumber += 1;
	}

	RunMessagePump();

	//Snap back to hold position every frame
	UpdateHoldPosition();
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::UpdateHoldPosition()
{
	if(!s_Instance)
	{
		return;
	}

	MouseState & state = s_Instance->m_mouseState;
	if(state.m_isHoldPositionActive)
	{
		Vector2i currentMousePosition = GetMousePosition();
		state.m_mouseHoldMovement = currentMousePosition - state.m_mouseHoldPosition;
		SetMousePosition(state.m_mouseHoldPosition);
	}
	else
	{
		state.m_mouseHoldMovement = Vector2i::ZERO;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetMouseHidden(bool hideMouse)
{
	if(!s_Instance)
	{
		return;
	}

	MouseState & state = s_Instance->m_mouseState;
	if(state.m_isHidden != hideMouse)
	{
		ShowCursor(hideMouse ? TRUE : FALSE);
		state.m_isHidden = hideMouse;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetMousePosition(Vector2i const & setPosition)
{
	if(!s_Instance)
	{
		return;
	}

	SetCursorPos(setPosition.x, setPosition.y);
	MouseState & state = s_Instance->m_mouseState;
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	state.m_mousePosition = setPosition;
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetMousePosition(int setX, int setY)
{
	SetMousePosition(Vector2i(setX, setY));
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::HoldMousePosition(Vector2i const & setPosition)
{
	if(!s_Instance)
	{
		return;
	}

	SetMousePosition(setPosition.x, setPosition.y);
	MouseState & state = s_Instance->m_mouseState;
	Vector2i m_mouseHoldDeviation;
	Vector2i m_mouseHoldPosition;
	state.m_isHoldPositionActive = true;
	state.m_mouseHoldMovement = Vector2i::ZERO;
	state.m_mouseHoldPosition = setPosition;
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::HoldMousePosition(int setX, int setY)
{
	HoldMousePosition(Vector2i(setX, setY));
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::ReleaseMouseHold()
{
	if(!s_Instance)
	{
		return;
	}

	MouseState & state = s_Instance->m_mouseState;
	state.m_mouseHoldPosition = Vector2i::ZERO;
	state.m_mouseHoldMovement = Vector2i::ZERO;
	state.m_isHoldPositionActive = false;
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetMouseButtonPressed(eMouseButton button)
{
	if(!s_Instance)
	{
		return;
	}

	ButtonState & state = s_Instance->m_mouseButtonStates[button];
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	state.m_isDown = true;

	NamedProperties mouseEvent;
	mouseEvent.Set(PARAM_MOUSE_BUTTON, button);
	BEventSystem::TriggerEvent(EVENT_MOUSE_DOWN, mouseEvent);
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetMouseButtonReleased(eMouseButton button)
{
	if(!s_Instance)
	{
		return;
	}

	ButtonState & state = s_Instance->m_mouseButtonStates[button];
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	state.m_isDown = false;

	NamedProperties mouseEvent;
	mouseEvent.Set(PARAM_MOUSE_BUTTON, button);
	BEventSystem::TriggerEvent(EVENT_MOUSE_UP, mouseEvent);
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetMouseWheelChanged(int value)
{
	if(!s_Instance)
	{
		return;
	}

	WheelState & state = s_Instance->m_mouseWheelState;
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	state.m_value = value;

	NamedProperties mouseEvent;
	mouseEvent.Set(PARAM_MOUSE_WHEEL, value);
	BEventSystem::TriggerEvent(EVENT_MOUSE_WHEEL, mouseEvent);
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetKeyPressed(KeyCode asKey)
{
	if(!s_Instance)
	{
		return;
	}

	ButtonState & state = s_Instance->m_keyStates[asKey];
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	if(!state.m_isDown)
	{
		state.m_isDown = true;

		NamedProperties keyEvent;
		keyEvent.Set(PARAM_KEY, asKey);
		BEventSystem::TriggerEvent(EVENT_KEY_DOWN, keyEvent);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetKeyReleased(KeyCode asKey)
{
	if(!s_Instance)
	{
		return;
	}

	ButtonState & state = s_Instance->m_keyStates[asKey];
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	if(state.m_isDown)
	{
		state.m_isDown = false;

		NamedProperties keyEvent;
		keyEvent.Set(PARAM_KEY, asKey);
		BEventSystem::TriggerEvent(EVENT_KEY_UP, keyEvent);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetFocusGained()
{
	if(!s_Instance)
	{
		return;
	}

	FocusState & state = s_Instance->m_focusState;
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	state.m_isFocused = false;

	BEventSystem::TriggerEvent(EVENT_FOCUS_GAINED);
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::SetFocusLost()
{
	if(!s_Instance)
	{
		return;
	}

	FocusState & state = s_Instance->m_focusState;
	state.m_frameChanged = s_Instance->m_currentFrameNumber;
	state.m_isFocused = false;

	BEventSystem::TriggerEvent(EVENT_FOCUS_LOST);
}


//-------------------------------------------------------------------------------------------------
STATIC void BMouseKeyboard::TypedCharacter(KeyCode asKey)
{
	if(!s_Instance)
	{
		return;
	}

	NamedProperties keyEvent;
	keyEvent.Set(PARAM_KEY, asKey);
	BEventSystem::TriggerEvent(EVENT_TYPED_CHAR, keyEvent);
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2i BMouseKeyboard::GetMousePosition(bool relativeToWindow /*= false*/)
{
	if(!s_Instance)
	{
		return Vector2i::ZERO;
	}

	POINT cursorPos;
	MouseState & mouseState = s_Instance->m_mouseState;
	Vector2i const windowDimensions = GetWindowDimensions();
	if(mouseState.m_frameChanged != s_Instance->m_currentFrameNumber)
	{
		GetCursorPos(&cursorPos);
		mouseState.m_mousePosition = Vector2i(cursorPos.x, cursorPos.y);
	}
	else
	{
		cursorPos.x = mouseState.m_mousePosition.x;
		cursorPos.y = mouseState.m_mousePosition.y;
	}

	if(relativeToWindow)
	{
		ScreenToClient(g_EngineSystem->GetWindowHandle(), &cursorPos);
	}

	//Invert Y because I want bottom to be 0 and top to be positive
	return Vector2i(cursorPos.x, windowDimensions.y - cursorPos.y);
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2i BMouseKeyboard::GetMouseHoldMovement()
{
	if(s_Instance)
	{
		MouseState & state = s_Instance->m_mouseState;
		return state.m_mouseHoldMovement;
	}

	return Vector2i::ZERO;
}


//-------------------------------------------------------------------------------------------------
STATIC bool BMouseKeyboard::GetMouseButtonDown(eMouseButton button)
{
	if(s_Instance)
	{
		ButtonState & state = s_Instance->m_mouseButtonStates[button];
		return state.m_isDown;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool BMouseKeyboard::IsMouseHidden()
{
	if(s_Instance)
	{
		MouseState & state = s_Instance->m_mouseState;
		return state.m_isHidden;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool BMouseKeyboard::GetKeyDown(KeyCode asKey)
{
	if(s_Instance)
	{
		ButtonState & state = s_Instance->m_keyStates[asKey];
		return state.m_isDown;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool BMouseKeyboard::IsFocused()
{
	if(s_Instance)
	{
		FocusState & state = s_Instance->m_focusState;
		return state.m_isFocused;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
BMouseKeyboard::BMouseKeyboard()
	: m_currentFrameNumber(0)
{
	memset(&m_mouseState, 0, sizeof(MouseState));
	memset(&m_mouseWheelState, 0, sizeof(WheelState));
	memset(&m_focusState, 0, sizeof(FocusState));
	for(int mouseIndex = 0; mouseIndex < eMouseButton_COUNT; ++mouseIndex)
	{
		m_mouseButtonStates[mouseIndex].m_frameChanged = 0;
		m_mouseButtonStates[mouseIndex].m_isDown = 0;
	}
	for(int keyIndex = 0; keyIndex < eKeyboardButton_COUNT; ++keyIndex)
	{
		m_keyStates[keyIndex].m_frameChanged = 0;
		m_keyStates[keyIndex].m_isDown = 0;
	}
}


//-------------------------------------------------------------------------------------------------
BMouseKeyboard::~BMouseKeyboard()
{
	//Nothing
}
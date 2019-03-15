#pragma once

#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
struct MouseState
{
public:
	bool m_isHidden;
	bool m_isHoldPositionActive;
	Vector2i m_mouseHoldMovement;
	Vector2i m_mouseHoldPosition;
	size_t m_frameChanged; // specifically used to not poll the mouse's position more than once a frame
	Vector2i m_mousePosition;
};


//-------------------------------------------------------------------------------------------------
struct WheelState
{
public:
	size_t m_frameChanged;
	int m_value;
};


//-------------------------------------------------------------------------------------------------
struct ButtonState
{
public:
	size_t m_frameChanged;
	bool m_isDown;
};


//-------------------------------------------------------------------------------------------------
struct FocusState
{
public:
	size_t m_frameChanged;
	bool m_isFocused;
};


//-------------------------------------------------------------------------------------------------
enum eMouseButton
{
	eMouseButton_LEFT,
	eMouseButton_MIDDLE,
	eMouseButton_RIGHT,

	eMouseButton_COUNT,
};


//-------------------------------------------------------------------------------------------------
enum eKeyboardButton
{
	eKeyboardButton_BACKSPACE	= 0x08,
	eKeyboardButton_TAB			= 0x09,
	eKeyboardButton_ENTER		= 0x0d,
	eKeyboardButton_SHIFT		= 0x10,
	eKeyboardButton_CTRL		= 0x11,
	eKeyboardButton_ESCAPE		= 0x1B,
	eKeyboardButton_LEFT		= 0x25,
	eKeyboardButton_UP			= 0x26,
	eKeyboardButton_RIGHT		= 0x27,
	eKeyboardButton_DOWN		= 0x28,
	eKeyboardButton_NUMPAD0		= 0x60,
	eKeyboardButton_NUMPAD1		= 0x61,
	eKeyboardButton_NUMPAD2		= 0x62,
	eKeyboardButton_NUMPAD3		= 0x63,
	eKeyboardButton_NUMPAD4		= 0x64,
	eKeyboardButton_NUMPAD5		= 0x65,
	eKeyboardButton_NUMPAD6		= 0x66,
	eKeyboardButton_NUMPAD7		= 0x67,
	eKeyboardButton_NUMPAD8		= 0x68,
	eKeyboardButton_NUMPAD9		= 0x69,
	eKeyboardButton_F5			= 0x74,
	eKeyboardButton_COLON		= 0xba,
	eKeyboardButton_COMMA		= 0xbc,
	eKeyboardButton_DASH		= 0xbd,
	eKeyboardButton_PERIOD		= 0xbe,
	eKeyboardButton_TILDE		= 0xc0,
	eKeyboardButton_PIPE		= 0xdc,

	eKeyboardButton_COUNT		= 0xff + 1,
};
typedef unsigned char KeyCode;


//-------------------------------------------------------------------------------------------------
class BMouseKeyboard
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static BMouseKeyboard * s_Instance;
	static char const * EVENT_TYPED_CHAR;
	static char const * EVENT_BACKSPACE_PRESSED;
	static char const * EVENT_KEY_DOWN;
	static char const * EVENT_KEY_UP;
	static char const * EVENT_MOUSE_WHEEL;
	static char const * EVENT_MOUSE_DOWN;
	static char const * EVENT_MOUSE_UP;
	static char const * EVENT_FOCUS_GAINED;
	static char const * EVENT_FOCUS_LOST;
	static char const * PARAM_KEY;
	static char const * PARAM_MOUSE_BUTTON;
	static char const * PARAM_MOUSE_WHEEL;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	size_t m_currentFrameNumber;
	MouseState m_mouseState;
	WheelState m_mouseWheelState;
	FocusState m_focusState;
	ButtonState m_mouseButtonStates[eMouseButton_COUNT];
	ButtonState m_keyStates[eKeyboardButton_COUNT];

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Update();
	static void UpdateHoldPosition();
	static void SetMouseHidden(bool hideMouse);
	static void SetMousePosition(Vector2i const & setPosition);
	static void SetMousePosition(int setX, int setY);
	static void HoldMousePosition(Vector2i const & setPosition);
	static void HoldMousePosition(int setX, int setY);
	static void ReleaseMouseHold();
	static void SetMouseButtonPressed(eMouseButton button);
	static void SetMouseButtonReleased(eMouseButton button);
	static void SetMouseWheelChanged(int value);
	static void SetKeyPressed(KeyCode asKey);
	static void SetKeyReleased(KeyCode asKey);
	static void SetFocusGained();
	static void SetFocusLost();
	static void TypedCharacter(KeyCode asKey);

	static Vector2i GetMousePosition(bool relativeToWindow = false);
	static Vector2i GetMouseHoldMovement();
	static bool GetMouseButtonDown(eMouseButton button);
	static bool IsMouseHidden();
	static bool GetKeyDown(KeyCode asKey);
	static bool IsFocused();

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BMouseKeyboard();
	~BMouseKeyboard();
};
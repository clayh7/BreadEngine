#pragma once

#include "Engine/Math/Vector2f.hpp"
#include <Windows.h>
#include <Xinput.h> // include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library


//-------------------------------------------------------------------------------------------------
//extern struct XINPUT_STATE;


//-------------------------------------------------------------------------------------------------
enum eXboxButton
{
	eXboxButton_Up,
	eXboxButton_Down,
	eXboxButton_Left,
	eXboxButton_Right,
	eXboxButton_Start,
	eXboxButton_Back,
	eXboxButton_LeftStickButton,
	eXboxButton_RightStickButton,
	eXboxButton_LeftBumper,
	eXboxButton_RightBumper,
	eXboxButton_Unused1,
	eXboxButton_Unused2,
	eXboxButton_A,
	eXboxButton_B,
	eXboxButton_X,
	eXboxButton_Y,

	eXboxButton_Count,
};


//-------------------------------------------------------------------------------------------------
enum eXboxTrigger
{
	eXboxTrigger_Left,
	eXboxTrigger_Right,

	eXboxTrigger_Count,
};


//-------------------------------------------------------------------------------------------------
enum eXboxStick
{
	eXboxStick_Left,
	eXboxStick_Right,
	
	eXboxStick_Count,
};


//-------------------------------------------------------------------------------------------------
enum eXboxPlayer
{
	eXboxPlayer_1,
	eXboxPlayer_2,
	eXboxPlayer_3,
	eXboxPlayer_4,
	
	eXboxPlayer_Count,
};


//-------------------------------------------------------------------------------------------------
class BXboxController
{
	//-------------------------------------------------------------------------------------------------
	// Structs
	//-------------------------------------------------------------------------------------------------
	struct ButtonState
	{
	public:
		size_t m_frameChanged;
		bool m_isDown;
	};


	//-------------------------------------------------------------------------------------------------
	struct TriggerState
	{
	public:
		size_t m_frameChanged;
		float m_value;
	};


	//-------------------------------------------------------------------------------------------------
	struct StickState
	{
	public:
		size_t m_frameChanged;
		float m_valueX;
		float m_valueY;
		float m_radians;
	};

	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static BXboxController * s_Instance;
	static const float STICK_DEADZONE_MIN;
	static const float STICK_DEADZONE_MAX;
	static const float HEAVY_VIBRATE_AMOUNT;
	static const float LIGHT_VIBRATE_AMOUNT;
	static char const * EVENT_XBOX_BUTTON;
	static char const * EVENT_XBOX_TRIGGER;
	static char const * EVENT_XBOX_STICK;
	static char const * PARAM_PLAYER;
	static char const * PARAM_XBOX_BUTTON;
	static char const * PARAM_XBOX_BUTTON_VALUE;
	static char const * PARAM_XBOX_TRIGGER;
	static char const * PARAM_XBOX_TRIGGER_VALUE;
	static char const * PARAM_XBOX_STICK;
	static char const * PARAM_XBOX_STICK_VALUE;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	size_t m_currentFrameNumber;
	XINPUT_STATE m_controllerState[eXboxPlayer_Count];
	bool m_isConnected[eXboxPlayer_Count];

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Update();
	static void Vibrate(int leftVal, int rightVal);
	static void Vibrate(int playerNum, int leftVal, int rightVal);

	static bool IsConnected(int playerNum = 0);
	static float GetTrigger(eXboxTrigger checkTrigger);
	static float GetTrigger(int playerNum, eXboxTrigger checkTrigger);
	static Vector2f GetStickPosition(eXboxStick checkStick);
	static Vector2f GetStickPosition(int playerNum, eXboxStick checkStick);
	static float GetStickRadians(eXboxStick checkStick);
	static float GetStickRadians(int playerNum, eXboxStick checkStick);

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BXboxController();
	~BXboxController();
};
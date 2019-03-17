#include "Engine/InputSystem/BXboxController.hpp"

#include <Math.h>
#include <Windows.h>
#include <Xinput.h> // include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"


//-------------------------------------------------------------------------------------------------
//XBOX stick reports values from 32767 to -32768
STATIC BXboxController * BXboxController::s_Instance = nullptr;
STATIC const float BXboxController::STICK_DEADZONE_MIN = 8500.f;
STATIC const float BXboxController::STICK_DEADZONE_MAX = 32000.f;
STATIC const float BXboxController::HEAVY_VIBRATE_AMOUNT = 60000.f;
STATIC const float BXboxController::LIGHT_VIBRATE_AMOUNT = 30000.f;
STATIC char const * BXboxController::EVENT_XBOX_BUTTON = "EventXboxButton";
STATIC char const * BXboxController::EVENT_XBOX_TRIGGER = "EventXboxTrigger";
STATIC char const * BXboxController::EVENT_XBOX_STICK = "EventXboxStick";
STATIC char const * BXboxController::PARAM_PLAYER = "Player";
STATIC char const * BXboxController::PARAM_XBOX_BUTTON = "XboxButton";
STATIC char const * BXboxController::PARAM_XBOX_BUTTON_VALUE = "XboxButtonValue";
STATIC char const * BXboxController::PARAM_XBOX_TRIGGER = "XboxTrigger";
STATIC char const * BXboxController::PARAM_XBOX_TRIGGER_VALUE = "XboxTriggerValue";
STATIC char const * BXboxController::PARAM_XBOX_STICK = "XboxStick";
STATIC char const * BXboxController::PARAM_XBOX_STICK_VALUE = "XboxStickValue";


//-------------------------------------------------------------------------------------------------
//unsigned int GetHexCode(eXboxButton button)
//{
//	return (1 << (button));
//}


//-------------------------------------------------------------------------------------------------
float ParseTriggerInput(BYTE trigger)
{
	return RangeMapNormalize(0, 255, trigger);
}


//-------------------------------------------------------------------------------------------------
float ParseStickRadiansInput(SHORT stickX, SHORT stickY)
{
	return (float)atan2(stickY, stickX);
}


//-------------------------------------------------------------------------------------------------
Vector2f ParseStickInput(SHORT stickX, SHORT stickY)
{
	float thetaRadians = ParseStickRadiansInput(stickY, stickX);
	float radius = (float)sqrt(stickY * stickY + stickX * stickX);
	float normalizedRadius = 0.f;

	if(radius < BXboxController::STICK_DEADZONE_MIN)
		normalizedRadius = 0.f;
	else if(radius > BXboxController::STICK_DEADZONE_MAX)
		normalizedRadius = 1.f;
	else
		normalizedRadius = RangeMapNormalize(BXboxController::STICK_DEADZONE_MIN, BXboxController::STICK_DEADZONE_MAX, radius);
	float valueX = normalizedRadius * cos(thetaRadians);
	float valueY = normalizedRadius * sin(thetaRadians);
	return Vector2f(valueX, valueY);
}


//-------------------------------------------------------------------------------------------------
STATIC void BXboxController::Update()
{
	if(!s_Instance)
	{
		return;
	}

	s_Instance->m_currentFrameNumber += 1;

	size_t buttonInputHex = 0;
	XINPUT_STATE currentXboxControllerState;
	memset(&currentXboxControllerState, 0, sizeof(XINPUT_STATE));
	for(int playerIndex = 0; playerIndex < eXboxPlayer_Count; ++playerIndex)
	{
		DWORD errorStatus = XInputGetState(playerIndex, &currentXboxControllerState);
		bool isConnected = (errorStatus == ERROR_SUCCESS);
		s_Instance->m_isConnected[playerIndex] = isConnected;
		if(isConnected)
		{
			XINPUT_STATE previousXboxControllerState = s_Instance->m_controllerState[playerIndex];
			s_Instance->m_controllerState[playerIndex] = currentXboxControllerState;

			// Trigger Button Events
			for(int buttonIndex = 0; buttonIndex < eXboxButton_Count; ++buttonIndex)
			{
				buttonInputHex = currentXboxControllerState.Gamepad.wButtons;
				bool currentButtonPressed = IsBitSet(buttonInputHex, buttonIndex);
				buttonInputHex = previousXboxControllerState.Gamepad.wButtons;
				bool previousButtonPressed = IsBitSet(buttonInputHex, buttonIndex);
				if(currentButtonPressed != previousButtonPressed)
				{
					//Pressed
					if(currentButtonPressed)
					{
						NamedProperties xboxEvent;
						xboxEvent.Set(PARAM_PLAYER, playerIndex);
						xboxEvent.Set(PARAM_XBOX_BUTTON, buttonIndex);
						xboxEvent.Set(PARAM_XBOX_BUTTON_VALUE, currentButtonPressed);
						BEventSystem::TriggerEvent(EVENT_XBOX_BUTTON, xboxEvent);
					}
					//Released
					else
					{
						NamedProperties xboxEvent;
						xboxEvent.Set(PARAM_PLAYER, playerIndex);
						xboxEvent.Set(PARAM_XBOX_BUTTON, buttonIndex);
						xboxEvent.Set(PARAM_XBOX_BUTTON_VALUE, currentButtonPressed);
						BEventSystem::TriggerEvent(EVENT_XBOX_BUTTON, xboxEvent);
					}
				}
			}

			// Trigger Trigger Events
			float leftTrigger = ParseTriggerInput(currentXboxControllerState.Gamepad.bLeftTrigger);
			float rightTrigger = ParseTriggerInput(currentXboxControllerState.Gamepad.bRightTrigger);

			NamedProperties xboxTriggerEvent;
			xboxTriggerEvent.Set(PARAM_PLAYER, playerIndex);
			xboxTriggerEvent.Set(PARAM_XBOX_TRIGGER, eXboxTrigger_Left);
			xboxTriggerEvent.Set(PARAM_XBOX_TRIGGER_VALUE, leftTrigger);
			BEventSystem::TriggerEvent(EVENT_XBOX_TRIGGER, xboxTriggerEvent);

			xboxTriggerEvent.Set(PARAM_PLAYER, playerIndex);
			xboxTriggerEvent.Set(PARAM_XBOX_TRIGGER, eXboxTrigger_Right);
			xboxTriggerEvent.Set(PARAM_XBOX_TRIGGER_VALUE, rightTrigger);
			BEventSystem::TriggerEvent(EVENT_XBOX_TRIGGER, xboxTriggerEvent);

			// Trigger Stick Events
			Vector2f leftStick = ParseStickInput(currentXboxControllerState.Gamepad.sThumbLX, currentXboxControllerState.Gamepad.sThumbLY);
			Vector2f rightStick = ParseStickInput(currentXboxControllerState.Gamepad.sThumbRX, currentXboxControllerState.Gamepad.sThumbRY);

			NamedProperties xboxStickEvent;
			xboxStickEvent.Set(PARAM_PLAYER, playerIndex);
			xboxStickEvent.Set(PARAM_XBOX_STICK, eXboxStick_Left);
			xboxStickEvent.Set(PARAM_XBOX_STICK_VALUE, leftStick);
			BEventSystem::TriggerEvent(EVENT_XBOX_STICK, xboxStickEvent);

			xboxStickEvent.Set(PARAM_PLAYER, playerIndex);
			xboxStickEvent.Set(PARAM_XBOX_STICK, eXboxStick_Right);
			xboxStickEvent.Set(PARAM_XBOX_STICK_VALUE, rightStick);
			BEventSystem::TriggerEvent(EVENT_XBOX_STICK, xboxStickEvent);
		}
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BXboxController::Vibrate(int leftVal, int rightVal)
{
	Vibrate(0, leftVal, rightVal);
}


//-------------------------------------------------------------------------------------------------
STATIC void BXboxController::Vibrate(int playerNum, int leftVal, int rightVal)
{
	// Create a Vibraton State
	XINPUT_VIBRATION Vibration;

	// Set the Vibration Values
	Vibration.wLeftMotorSpeed = (WORD)leftVal;
	Vibration.wRightMotorSpeed = (WORD)rightVal;

	// Vibrate the controller
	XInputSetState((DWORD)playerNum, &Vibration);
}


//-------------------------------------------------------------------------------------------------
STATIC bool BXboxController::IsConnected(int playerNum /*= 0*/)
{
	if(!s_Instance)
	{
		return false;
	}

	return s_Instance->m_isConnected[playerNum];
}


//-------------------------------------------------------------------------------------------------
STATIC float BXboxController::GetTrigger(eXboxTrigger checkTrigger)
{
	return GetTrigger(0, checkTrigger);
}


//-------------------------------------------------------------------------------------------------
STATIC float BXboxController::GetTrigger(int playerNum, eXboxTrigger checkTrigger)
{
	if(!s_Instance)
	{
		return 0.f;
	}

	if(checkTrigger == eXboxTrigger_Left)
	{
		return ParseTriggerInput(s_Instance->m_controllerState[playerNum].Gamepad.bLeftTrigger);
	}
	else
	{
		return ParseTriggerInput(s_Instance->m_controllerState[playerNum].Gamepad.bRightTrigger);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2f BXboxController::GetStickPosition(eXboxStick checkStick)
{
	return GetStickPosition(0, checkStick);
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2f BXboxController::GetStickPosition(int playerNum, eXboxStick checkStick)
{
	if(!s_Instance)
	{
		return Vector2f::ZERO;
	}

	if(checkStick == eXboxStick_Left)
	{
		return ParseStickInput(s_Instance->m_controllerState[playerNum].Gamepad.sThumbLX, s_Instance->m_controllerState[playerNum].Gamepad.sThumbLY);
	}
	else
	{
		return ParseStickInput(s_Instance->m_controllerState[playerNum].Gamepad.sThumbRX, s_Instance->m_controllerState[playerNum].Gamepad.sThumbRY);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC float BXboxController::GetStickRadians(eXboxStick checkStick)
{
	return GetStickRadians(0, checkStick);
}


//-------------------------------------------------------------------------------------------------
STATIC float BXboxController::GetStickRadians(int playerNum, eXboxStick checkStick)
{
	if(!s_Instance)
	{
		return 0.f;
	}

	if(checkStick == eXboxStick_Left)
	{
		return ParseStickRadiansInput(s_Instance->m_controllerState[playerNum].Gamepad.sThumbLX, s_Instance->m_controllerState[playerNum].Gamepad.sThumbLY);
	}
	else
	{
		return ParseStickRadiansInput(s_Instance->m_controllerState[playerNum].Gamepad.sThumbRX, s_Instance->m_controllerState[playerNum].Gamepad.sThumbRY);
	}
}


//-------------------------------------------------------------------------------------------------
BXboxController::BXboxController()
	: m_currentFrameNumber(0)
{
	for(int playerIndex = 0; playerIndex < eXboxPlayer_Count; ++playerIndex)
	{
		memset(&m_controllerState[playerIndex], 0, sizeof(XINPUT_STATE));
		m_isConnected[playerIndex] = false;
	}
}


//-------------------------------------------------------------------------------------------------
BXboxController::~BXboxController()
{
	//Nothing
}
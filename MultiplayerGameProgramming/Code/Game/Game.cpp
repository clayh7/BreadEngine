#include "Game/Game.hpp"

#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/InputSystem/BMouseKeyboard.hpp"
#include "Level.hpp"


//-------------------------------------------------------------------------------------------------
Game * g_GameSystem = nullptr;

//-------------------------------------------------------------------------------------------------
Game::Game()
	: m_currentLevel(std::shared_ptr<Level>(new Level(5, 5)))
{
	m_currentLevel->SetupDemo();
	BEventSystem::RegisterEvent(BMouseKeyboard::EVENT_KEY_DOWN, this, &Game::OnKeyPressed);
}


//-------------------------------------------------------------------------------------------------
Game::~Game()
{
	BEventSystem::TriggerEvent(EVENT_GAME_SHUTDOWN);
}


//-------------------------------------------------------------------------------------------------
void Game::OnKeyPressed(NamedProperties& params)
{
	KeyCode key;
	params.Get(BMouseKeyboard::PARAM_KEY, key);

	switch (key)
	{
	case 'S':
		BConsoleSystem::AddLog("Test");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void Game::Update()
{
	BEventSystem::TriggerEvent(EVENT_GAME_UPDATE);
}


//-------------------------------------------------------------------------------------------------
void Game::Render() const
{
	BEventSystem::TriggerEvent(EVENT_GAME_RENDER);
}
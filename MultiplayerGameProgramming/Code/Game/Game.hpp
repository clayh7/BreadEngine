#pragma once


//-------------------------------------------------------------------------------------------------
class Game;


//-------------------------------------------------------------------------------------------------
extern Game * g_GameSystem;


//-------------------------------------------------------------------------------------------------
class Game
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	Game();
	~Game();
	void Update();
	void Render() const;
};
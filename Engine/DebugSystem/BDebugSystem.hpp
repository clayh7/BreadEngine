#pragma once

#include <vector>


//-------------------------------------------------------------------------------------------------
class BDebugSystem;
class TextRenderer;
class Command;


//-------------------------------------------------------------------------------------------------
void DebugFPSCommand(Command const &);
void DebugUnitCommand(Command const &);
void DebugMemoryCommand(Command const &);
void DebugFlushCommand(Command const &);


//-------------------------------------------------------------------------------------------------
class BDebugSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static BDebugSystem * s_System;

private:
	static float const DEBUG_LINE_SPACING;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	bool m_showFPSDebug;
	bool m_showUnitDebug;
	bool m_showMemoryDebug;
	int m_lineCount;
	std::vector<TextRenderer*> m_debugTexts;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void Update();
	static void Render();
	static BDebugSystem * CreateOrGetSystem();

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BDebugSystem();
	~BDebugSystem();
	void SystemUpdate();
	void UpdateTextFPS(int & currentLine);
	void UpdateTextUnit(int & currentLine);
	void UpdateTextMemory(int & currentLine);
	void UpdateTextMemoryVerbose(int & currentLine);
	void ClearTextRemaining(int & currentLine);
	void SystemRender() const;

	void ToggleDebugFPS();
	void ToggleDebugUnit();
	void ToggleDebugMemory();
	void DebugFlushMemory();
};
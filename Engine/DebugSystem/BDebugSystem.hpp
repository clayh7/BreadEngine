#pragma once

#include <vector>


//-------------------------------------------------------------------------------------------------
class BDebugSystem;
class TextRenderer;
class Command;


//-------------------------------------------------------------------------------------------------
extern BDebugSystem * g_DebugSystem;


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
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BDebugSystem();
	~BDebugSystem();
	void Update();
	void UpdateTextFPS(int & currentLine);
	void UpdateTextUnit(int & currentLine);
	void UpdateTextMemory(int & currentLine);
	void UpdateTextMemoryVerbose(int & currentLine);
	void ClearTextRemaining(int & currentLine);
	void Render() const;

	void ToggleDebugFPS();
	void ToggleDebugUnit();
	void ToggleDebugMemory();
	void DebugFlushMemory();
};
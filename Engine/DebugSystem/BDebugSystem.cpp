#include "Engine/DebugSystem/BDebugSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/DebugSystem/Logger.hpp"
#include "Engine/InputSystem/BInputSystem.hpp"
#include "Engine/Math/Vector2i.hpp"
#include "Engine/MemorySystem/BMemorySystem.hpp"
#include "Engine/Utils/NetworkUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC float const BDebugSystem::DEBUG_LINE_SPACING = 25.f;


//-------------------------------------------------------------------------------------------------
STATIC BDebugSystem * BDebugSystem::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
void DebugFPSCommand(Command const &)
{
	BConsoleSystem::AddLog("Toggling FPS debug.", BConsoleSystem::GOOD);
	if(BDebugSystem::s_System)
	{
		BDebugSystem::s_System->ToggleDebugFPS();
	}
}


//-------------------------------------------------------------------------------------------------
void DebugUnitCommand(Command const &)
{
	BConsoleSystem::AddLog("Toggling unit debug.", BConsoleSystem::GOOD);
	if(BDebugSystem::s_System)
	{
		BDebugSystem::s_System->ToggleDebugUnit();
	}
}


//-------------------------------------------------------------------------------------------------
void DebugMemoryCommand(Command const &)
{
	BConsoleSystem::AddLog("Toggling memory debug.", BConsoleSystem::GOOD);
	if(BDebugSystem::s_System)
	{
		BDebugSystem::s_System->ToggleDebugMemory();
	}
}


//-------------------------------------------------------------------------------------------------
void DebugFlushCommand(Command const &)
{
	BConsoleSystem::AddLog("Flushing memory callstack.", BConsoleSystem::GOOD);
	if(BDebugSystem::s_System)
	{
		BDebugSystem::s_System->DebugFlushMemory();
	}
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::Startup()
{
	if(!s_System)
	{
		s_System = new BDebugSystem();
	}
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::Shutdown()
{
	if(s_System)
	{
		delete s_System;
		s_System = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
BDebugSystem * BDebugSystem::CreateOrGetSystem()
{
	if(!s_System)
	{
		Startup();
	}

	return s_System;
}


//-------------------------------------------------------------------------------------------------
BDebugSystem::BDebugSystem()
	: m_showFPSDebug(false)
	, m_showUnitDebug(false)
	, m_showMemoryDebug(false)
	, m_lineCount(14)
{
	BConsoleSystem::Register("debug_fps", &DebugFPSCommand, " : Show/Hide FPS info.");
	BConsoleSystem::Register("debug_unit", &DebugUnitCommand, " : Show/Hide frame breakdown info.");

	Vector2i windowDimensions = GetWindowDimensions();
	float topOfWindow = static_cast<float>(windowDimensions.y - 15.f);

	for(int index = 0; index < m_lineCount; ++index)
	{
		Vector2f linePosition = Vector2f(10.f, topOfWindow - DEBUG_LINE_SPACING * index);
		m_debugTexts.push_back(new TextRenderer(" ", linePosition));
	}

	//#TODO: this?
	//LoggingSystem = new Logger( );
	//LoggingSystem->Begin( );

	BProfiler::Startup();

#if MEMORY_TRACKING >= 1
	BConsoleSystem::Register("debug_memory", &DebugMemoryCommand, " : Show/Hide memory allocation info.");
	BConsoleSystem::Register("debug_flush", &DebugFlushCommand, " : Print memory callstack to the debug log.");
#endif // MEMORY_TRACKING >= 1

	BEventSystem::RegisterEvent(EVENT_ENGINE_UPDATE, this, &BDebugSystem::OnUpdate);
	BEventSystem::RegisterEvent(EVENT_ENGINE_RENDER, this, &BDebugSystem::OnRender);
}


//-------------------------------------------------------------------------------------------------
BDebugSystem::~BDebugSystem()
{
	BEventSystem::Unregister(this);

	for(TextRenderer * textRenderer : m_debugTexts)
	{
		delete textRenderer;
		textRenderer = nullptr;
	}
	m_debugTexts.clear();

	BProfiler::Shutdown();

	delete LoggingSystem;
	LoggingSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::OnUpdate(NamedProperties &)
{
	int currentLine = 0;

	if(m_showFPSDebug)
	{
		UpdateTextFPS(currentLine);
	}

	//#TODO: Currently handled by the Profiler, may update this later to make it like UE4
	if(m_showUnitDebug)
	{
		//UpdateTextUnit( currentLine );
	}

#if MEMORY_TRACKING >= 1
	if(m_showMemoryDebug)
	{
		UpdateTextMemory(currentLine);
	}
#endif // MEMORY_TRACKING >= 1

#if MEMORY_TRACKING >= 2
	if(m_showMemoryDebug)
	{
		UpdateTextMemoryVerbose(currentLine);
	}
#endif // MEMORY_TRACKING >= 2

	ClearTextRemaining(currentLine);
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::UpdateTextFPS(int & currentLine)
{
	float fps = 1.f / Time::DELTA_SECONDS;
	std::string debugText = Stringf("FPS: %.1f", fps);
	Color fpsColor = Color::WHITE;
	if(fps < 30.0f)
	{
		fpsColor = Color::RED;
	}
	else if(fps < 45.0f)
	{
		fpsColor = Color::ORANGE;
	}
	else if(fps < 55.0f)
	{
		fpsColor = Color::YELLOW;
	}
	m_debugTexts[currentLine]->SetText(debugText);
	m_debugTexts[currentLine]->SetColor(fpsColor);
	m_debugTexts[currentLine]->Update();
	currentLine += 1;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::UpdateTextUnit(int & currentLine)
{
	float gameTime = 2.0f;
	std::string debugText = Stringf("Game: %.1fms", gameTime);
	Color unitColor = Color::WHITE;
	if(gameTime > 16.0f)
	{
		unitColor = Color::RED;
	}
	else if(gameTime > 8.0f)
	{
		unitColor = Color::YELLOW;
	}
	m_debugTexts[currentLine]->SetText(debugText);
	m_debugTexts[currentLine]->SetColor(unitColor);
	m_debugTexts[currentLine]->Update();
	currentLine += 1;

	float renderTime = 2.5f;
	debugText = Stringf("Render: %.1fms", renderTime);
	unitColor = Color::WHITE;
	if(gameTime > 16.0f)
	{
		unitColor = Color::RED;
	}
	else if(gameTime > 8.0f)
	{
		unitColor = Color::YELLOW;
	}
	m_debugTexts[currentLine]->SetText(debugText);
	m_debugTexts[currentLine]->SetColor(unitColor);
	m_debugTexts[currentLine]->Update();
	currentLine += 1;

	BRenderSystem * RSystem = BRenderSystem::s_System;
	debugText = Stringf("Draw: %d", RSystem ? RSystem->m_currentDrawCalls : 0);
	m_debugTexts[currentLine]->SetText(debugText);
	m_debugTexts[currentLine]->SetColor(Color::WHITE);
	m_debugTexts[currentLine]->Update();
	currentLine += 1;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::UpdateTextMemory(int & currentLine)
{
	std::string debugText;
	BMemorySystem::GetMemoryAllocationsString(debugText);
	m_debugTexts[currentLine]->SetText(debugText);
	m_debugTexts[currentLine]->SetColor(Color::WHITE);
	m_debugTexts[currentLine]->Update();
	currentLine += 1;

	BMemorySystem::GetMemoryAveragesString(debugText);
	m_debugTexts[currentLine]->SetText(debugText);
	m_debugTexts[currentLine]->SetColor(Color::WHITE);
	m_debugTexts[currentLine]->Update();
	currentLine += 1;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::UpdateTextMemoryVerbose(int & currentLine)
{
	//Get the stats
	BMemorySystem * MSystem = BMemorySystem::GetOrCreateSystem();
	UntrackedCallstackStatsMap & callstackStatsMap = MSystem->m_callstackStatsMap;

	//Order them base on allocation size
	std::map<size_t, CallstackStats> orderedStats;
	for(auto callstackStatsItem : callstackStatsMap)
	{
		orderedStats.insert(std::pair<size_t, CallstackStats>(callstackStatsItem.second.m_totalBytes, callstackStatsItem.second));
	}

	for(auto callstackStatsIter = orderedStats.end(); callstackStatsIter != orderedStats.begin(); )
	{
		--callstackStatsIter;
		if(currentLine + 2 <= m_lineCount)
		{
			CallstackStats & callstackStats = callstackStatsIter->second;
			std::string stats = Stringf("Allocations: %u | Bytes: %u", callstackStats.m_totalAllocations, callstackStats.m_totalBytes);
			std::string line = callstackStats.m_lineAndNumber;
			m_debugTexts[currentLine]->SetText(stats);
			m_debugTexts[currentLine]->SetColor(Color::WHITE);
			m_debugTexts[currentLine]->Update();
			currentLine += 1;

			m_debugTexts[currentLine]->SetText(line);
			m_debugTexts[currentLine]->SetColor(Color::WHITE);
			m_debugTexts[currentLine]->Update();
			currentLine += 1;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::ClearTextRemaining(int & currentLine)
{
	//Clear the rest
	while(currentLine < m_lineCount)
	{
		m_debugTexts[currentLine]->SetText(" ");
		m_debugTexts[currentLine]->Update();
		currentLine += 1;
	}
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::OnRender(NamedProperties &) const
{
	if(m_showFPSDebug || m_showUnitDebug || m_showMemoryDebug)
	{
		for(size_t memDebugIndex = 0; memDebugIndex < m_debugTexts.size(); ++memDebugIndex)
		{
			m_debugTexts[memDebugIndex]->Render();
		}
	}

	BProfiler::Render();
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::ToggleDebugFPS()
{
	m_showFPSDebug = !m_showFPSDebug;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::ToggleDebugUnit()
{
	m_showUnitDebug = !m_showUnitDebug;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::ToggleDebugMemory()
{
	m_showMemoryDebug = !m_showMemoryDebug;
}


//-------------------------------------------------------------------------------------------------
void BDebugSystem::DebugFlushMemory()
{
	BMemorySystem::GetOrCreateSystem()->Flush();
}
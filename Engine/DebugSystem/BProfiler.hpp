#pragma once

#include <map>
#include <vector>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"


//-------------------------------------------------------------------------------------------------
class BitmapFont;
class BProfiler;
class BProfilerReport;
class BProfilerSample;
class Command;


//-------------------------------------------------------------------------------------------------
void PingProfilerCommand(Command const &);
void EnableProfilerCommand(Command const &);
void DisableProfilerCommand(Command const &);


//-------------------------------------------------------------------------------------------------
class BProfiler
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static int const POOL_SIZE = 100000;
	static int const LINE_COUNT = 40;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	static char const * ROOT_SAMPLE;
	static bool s_initialized;
	static bool s_toBeEnabled;
	static bool s_toBePinged;
	static bool s_enabled;
	static bool s_reportList;
	static bool s_show;
	static BitmapFont * s_profilerFont;
	static uint64_t s_previousFrameTotalTime;
	static BProfilerSample * s_currentSample;
	static BProfilerSample * s_currentSampleSet;
	static BProfilerSample * s_previousSampleSet;
	static ObjectPool<BProfilerSample> s_samplePool;
	static std::vector<TextRenderer*, UntrackedAllocator<TextRenderer*>> s_profilerLines;

	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void Update();
	static void Render();
	static void StartSample(char const * sampleTag);
	static void StopSample();
	static void SetEnabled(bool isEnabled);
	static void SetPing();
	static void ToggleLayout();
	static void SetProfilerVisible(bool show);
	static void IncrementNews();
	static void IncrementDeletes();
	static void Delete(BProfilerSample * sample);
	static bool IsEnabled();

private:
	static void RenderReport(std::multimap<int, BProfilerReport> const & report);
};
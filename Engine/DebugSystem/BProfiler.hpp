#pragma once

#include <map>
#include <vector>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/MemorySystem/ObjectPool.hpp"
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
	static char const * ROOT_SAMPLE;
	static BProfiler * s_Instance;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	bool m_toBeEnabled;
	bool m_toBePinged;
	bool m_enabled;
	bool m_reportList;
	bool m_show;
	BitmapFont * m_profilerFont;
	uint64_t m_previousFrameTotalTime;
	BProfilerSample * m_currentSample;
	BProfilerSample * m_currentSampleSet;
	BProfilerSample * m_previousSampleSet;
	ObjectPool<BProfilerSample> m_samplePool;
	std::vector<TextRenderer*> m_profilerLines;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void Update();
	static void Render();
	static BProfiler * CreateOrGetInstance();
	static void StartSample(char const * sampleTag);
	static void StopSample();

private:

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BProfiler();
	~BProfiler();
	void InstanceUpdate();
	void InstanceRender();
	void RenderReport(std::multimap<int, BProfilerReport> const & report);
	void InstanceStartSample(char const * sampleTag);
	void InstanceStopSample();
	void SetEnabled(bool isEnabled);
	void SetPing();
	void ToggleLayout();
	void SetProfilerVisible(bool show);
	void IncrementNews();
	void IncrementDeletes();
	void Delete(BProfilerSample * sample);
	bool IsEnabled();
};
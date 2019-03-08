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
	static BProfiler * s_ProfilerSystem;
	static char const * ROOT_SAMPLE;
	static int const POOL_SIZE = 100000;
	static int const LINE_COUNT = 40;

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
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static void Initialize();
	static void Destroy();
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

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
private:
	BProfiler();
	~BProfiler();

	void GenerateListReport(BProfilerSample const & sample, int & index, int depth, std::multimap<int, BProfilerReport> & report) const;
	void GenerateFlatReport(BProfilerSample const & sample, std::multimap<int, BProfilerReport> & report) const;
	void RenderReport(std::multimap<int, BProfilerReport> const & report) const;

	void UpdateInput();
	void FrameMark();
};
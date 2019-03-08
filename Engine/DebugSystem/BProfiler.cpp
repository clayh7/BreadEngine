#include "Engine/DebugSystem/BProfiler.hpp"

#include <set>
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfilerReport.hpp"
#include "Engine/DebugSystem/BProfilerSample.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/RenderSystem/BitmapFont.hpp"
#include "Engine/Utils/StringUtils.hpp"



//-------------------------------------------------------------------------------------------------
BProfiler * BProfiler::s_ProfilerSystem = nullptr;
char const * BProfiler::ROOT_SAMPLE = "FRAME";


//-------------------------------------------------------------------------------------------------
void PingProfilerCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler pinged.", Console::GOOD);
	BProfiler::SetPing();
	BProfiler::SetProfilerVisible(true);
}


//-------------------------------------------------------------------------------------------------
void EnableProfilerCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler enabled.", Console::GOOD);
	BProfiler::SetEnabled(true);
	BProfiler::SetProfilerVisible(true);
}


//-------------------------------------------------------------------------------------------------
void DisableProfilerCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler disabled.", Console::GOOD);
	BProfiler::SetEnabled(false);
	BProfiler::SetProfilerVisible(false);
}


//-------------------------------------------------------------------------------------------------
void ToggleReportLayoutCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler layout changed.", Console::GOOD);
	BProfiler::ToggleLayout();
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Initialize()
{
#ifdef DEBUG_PROFILER
	if (!s_ProfilerSystem)
	{
		s_ProfilerSystem = new BProfiler();
	}
#endif
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Destroy()
{
	delete s_ProfilerSystem;
	s_ProfilerSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Update()
{
#ifdef DEBUG_PROFILER
	if (!s_ProfilerSystem)
	{
		s_ProfilerSystem->UpdateInput();
		s_ProfilerSystem->FrameMark();
	}
#endif
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Render()
{
#ifdef DEBUG_PROFILER
	if (!s_ProfilerSystem)
	{
		return;
	}

	if (!s_ProfilerSystem->m_show)
	{
		return;
	}

	if (!s_ProfilerSystem->m_previousSampleSet)
	{
		return;
	}

	//Render previous frame's sample set
	if (s_ProfilerSystem->m_reportList)
	{
		int line = 0;
		std::multimap<int, BProfilerReport> listReport;
		s_ProfilerSystem->GenerateListReport(*s_ProfilerSystem->m_previousSampleSet, line, 0, listReport);
		s_ProfilerSystem->RenderReport(listReport);
	}
	else
	{
		std::multimap<int, BProfilerReport> flatReport;
		s_ProfilerSystem->GenerateFlatReport(*s_ProfilerSystem->m_previousSampleSet, flatReport);
		s_ProfilerSystem->RenderReport(flatReport);
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::StartSample(char const * sampleTag)
{
#ifdef DEBUG_PROFILER
	if (!s_ProfilerSystem || !s_ProfilerSystem->m_enabled)
	{
		return;
	}

	BProfilerSample * previousSample = s_ProfilerSystem->m_currentSample;
	if (previousSample)
	{
		s_ProfilerSystem->m_currentSample = s_ProfilerSystem->m_samplePool.Alloc();
		s_ProfilerSystem->m_currentSample->SetTag(sampleTag);
		s_ProfilerSystem->m_currentSample->parent = previousSample;

		previousSample->AddChild(s_ProfilerSystem->m_currentSample);
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::StopSample()
{
#ifdef DEBUG_PROFILER
	if (!s_ProfilerSystem || !s_ProfilerSystem->m_enabled)
	{
		return;
	}

	if (s_ProfilerSystem->m_currentSample)
	{
		s_ProfilerSystem->m_currentSample->opCountEnd = Time::GetCurrentOpCount();
		s_ProfilerSystem->m_currentSample = s_ProfilerSystem->m_currentSample->parent;
	}
	else
	{
		ASSERT_RECOVERABLE(s_ProfilerSystem->m_currentSample, "No current sample in profiler, too many StopSample()'s");
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::SetEnabled(bool isEnabled)
{
	if (s_ProfilerSystem)
	{
		s_ProfilerSystem->m_toBeEnabled = isEnabled;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::SetPing()
{
	if (s_ProfilerSystem)
	{
		s_ProfilerSystem->m_toBePinged = true;
		s_ProfilerSystem->m_toBeEnabled = true;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::ToggleLayout()
{
	if (s_ProfilerSystem)
	{
		s_ProfilerSystem->m_reportList = !s_ProfilerSystem->m_reportList;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::SetProfilerVisible(bool show)
{
	if (s_ProfilerSystem)
	{
		s_ProfilerSystem->m_show = show;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::IncrementNews()
{
	if (s_ProfilerSystem && s_ProfilerSystem->m_currentSample)
	{
		s_ProfilerSystem->m_currentSample->newCount++;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::IncrementDeletes()
{
	if (s_ProfilerSystem && s_ProfilerSystem->m_currentSample)
	{
		s_ProfilerSystem->m_currentSample->deleteCount++;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Delete(BProfilerSample * sample)
{
	if (s_ProfilerSystem)
	{
		s_ProfilerSystem->m_samplePool.Delete(sample);
	}
}


//-------------------------------------------------------------------------------------------------
bool BProfiler::IsEnabled()
{
	return (s_ProfilerSystem && s_ProfilerSystem->m_enabled);
}


//-------------------------------------------------------------------------------------------------
BProfiler::BProfiler()
	: m_toBeEnabled(false)
	, m_toBePinged(false)
	, m_enabled(false)
	, m_reportList(false)
	, m_show(true)
	, m_profilerFont(nullptr)
	, m_previousFrameTotalTime(0)
	, m_currentSample(nullptr)
	, m_currentSampleSet(nullptr)
	, m_previousSampleSet(nullptr)
	, m_samplePool(POOL_SIZE)
{
#ifdef DEBUG_PROFILER
	m_profilerFont = BitmapFont::CreateOrGetFont("Data/Fonts/ClayFont.png");
	g_ConsoleSystem->RegisterCommand("profiler_enable", EnableProfilerCommand, " : Tracks samples injected in code and reports on screen.");
	g_ConsoleSystem->RegisterCommand("profiler_disable", DisableProfilerCommand, " : Stops tracking samples injected in code and reports on screen.");
	g_ConsoleSystem->RegisterCommand("profiler_ping", PingProfilerCommand, " : Tracks one sample.");
	g_ConsoleSystem->RegisterCommand("profiler_layout", ToggleReportLayoutCommand, " : Change the profiler's report layout. List/Flat");
	for (int lineIndex = 0; lineIndex < LINE_COUNT; ++lineIndex)
	{
		TextRenderer * profilerLine = new TextRenderer("", Vector2f(50.f, 850.f - 20.f * lineIndex), 13, m_profilerFont);
		m_profilerLines.push_back(profilerLine);
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
BProfiler::~BProfiler()
{
#ifdef DEBUG_PROFILER
	m_currentSample = nullptr;
	m_samplePool.Destroy();

	for (int lineIndex = 0; lineIndex < LINE_COUNT; ++lineIndex)
	{
		delete m_profilerLines[lineIndex];
		m_profilerLines[lineIndex] = nullptr;
	}
	m_profilerLines.clear();
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void BProfiler::GenerateListReport(BProfilerSample const & sample, int & index, int depth, std::multimap<int, BProfilerReport> & report) const
{
#ifdef DEBUG_PROFILER
	BProfilerReport reportNode;
	reportNode.index = index;
	reportNode.tag = sample.tag;
	reportNode.depth = depth;
	reportNode.totalTime = sample.opCountEnd - sample.opCountStart;
	reportNode.childrenTime = 0;
	reportNode.newCount = sample.newCount;
	reportNode.deleteCount = sample.deleteCount;
	BProfilerSample * child = sample.children;
	while (child)
	{
		index += 1;
		reportNode.childrenTime += (child->opCountEnd - child->opCountStart);
		GenerateListReport(*child, index, depth + 1, report);
		child = child->next;
	}
	reportNode.selfTime = reportNode.totalTime - reportNode.childrenTime;
	reportNode.percent = ((float)((reportNode.totalTime * 1000) / m_previousFrameTotalTime)) / 10.f;
	report.insert(std::pair<int, BProfilerReport>(reportNode.index, reportNode));
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void PopulateTagList(BProfilerSample const & sample, std::set<char const *> & tags)
{
#ifdef DEBUG_PROFILER
	tags.insert(sample.tag);
	BProfilerSample * child = sample.children;
	while (child)
	{
		PopulateTagList(*child, tags);
		child = child->next;
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void PopulateReportFromSample(BProfilerReport & reportData, BProfilerSample const & sample)
{
#ifdef DEBUG_PROFILER
	//Add contents
	if (reportData.tag == sample.tag)
	{
		reportData.newCount += sample.newCount;
		reportData.deleteCount += sample.deleteCount;
		reportData.totalTime += sample.opCountEnd - sample.opCountStart;
		BProfilerSample * checkChild = sample.children;
		while (checkChild)
		{
			reportData.childrenTime += (checkChild->opCountEnd - checkChild->opCountStart);
			checkChild = checkChild->next;
		}
		reportData.selfTime = reportData.totalTime - reportData.childrenTime;
	}

	//Continue down tree
	BProfilerSample * child = sample.children;
	while (child)
	{
		PopulateReportFromSample(reportData, *child);
		child = child->next;
	}
#endif // DEBUG_PROFILER
}

//-------------------------------------------------------------------------------------------------
void BProfiler::GenerateFlatReport(BProfilerSample const & sample, std::multimap<int, BProfilerReport> & report) const
{
#ifdef DEBUG_PROFILER
	std::set<char const *> tagNames;
	PopulateTagList(sample, tagNames);
	for (auto tagName : tagNames)
	{
		BProfilerReport currentReport;
		currentReport.tag = tagName;
		PopulateReportFromSample(currentReport, sample);
		currentReport.percent = ((float)((currentReport.selfTime * 1000) / m_previousFrameTotalTime)) / 10.f;
		//This is to sort it by percent, lowest number is sorted to the top
		report.insert(std::pair<int, BProfilerReport>(-(int)(currentReport.percent * 100.f), currentReport));
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void BProfiler::RenderReport(std::multimap<int, BProfilerReport> const & report) const
{
#ifdef DEBUG_PROFILER
	m_profilerLines[0]->SetText(Stringf("TAG                  TIME      SELF TIME  PERCENT  NEW   DELETE"));
	m_profilerLines[0]->Update();
	m_profilerLines[0]->Render();
	int count = 1;
	for (auto reportData : report)
	{
		BProfilerReport const & profilerReport = reportData.second;
		double timeSeconds = Time::GetTimeFromOpCount(profilerReport.totalTime);
		double selfTimeSeconds = Time::GetTimeFromOpCount(profilerReport.selfTime);
		std::string tag = Stringf("%.*s%s", profilerReport.depth, "--------------------", profilerReport.tag);
		m_profilerLines[count]->SetText(Stringf("%-20s %06.2fms  %06.2fms   %5.1f%%   %-6d%-6d", tag.c_str(), timeSeconds*1000.0, selfTimeSeconds*1000.0, profilerReport.percent, profilerReport.newCount, profilerReport.deleteCount));
		m_profilerLines[count]->Update();
		m_profilerLines[count]->Render();
		count += 1;
		if (count >= LINE_COUNT)
		{
			return;
		}
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void BProfiler::UpdateInput()
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void BProfiler::FrameMark()
{
#ifdef DEBUG_PROFILER
	if (m_enabled)
	{
		ASSERT_RECOVERABLE(m_currentSample == m_currentSampleSet, "Current Sample is not the root sample");

		//Clear last Sample Set
		if (m_previousSampleSet)
		{
			Delete(m_previousSampleSet);
		}
		if (m_currentSample)
		{
			StopSample();
		}
		//Swap current sample set to previous
		m_previousSampleSet = m_currentSampleSet;
		if (m_previousSampleSet)
		{
			m_previousFrameTotalTime = m_previousSampleSet->opCountEnd - m_previousSampleSet->opCountStart;
		}
	}
	m_enabled = m_toBeEnabled;
	if (m_enabled)
	{
		StartSample(ROOT_SAMPLE);
		m_currentSampleSet = m_currentSample;
	}
	//Ping only one frame
	if (m_toBePinged)
	{
		m_toBeEnabled = false;
		m_toBePinged = false;
	}
#endif // DEBUG_PROFILER
}
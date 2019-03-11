//-------------------------------------------------------------------------------------------------

//CHANGE DEBUGGING > WORKING DIRECTORY > $(SolutionDir)Run_$(PlatformName)/

//-------------------------------------------------------------------------------------------------

#define DISABLE_ASSERTS 0

// 0 - Asserts active
// 1 - Asserts disabled

//-------------------------------------------------------------------------------------------------

#define APP_NAME "ProtoGame" //Name on top game window

//-------------------------------------------------------------------------------------------------

// MEMORY_TRACKING - Tracks memory allocations on the heap
// Console Commands: debug_memory
// (Default = 1)

#define MEMORY_TRACKING 2

// Level 0 - Use this to turn off tracking
// Level 1 - Counts allocations and deallocations
// Level 2 - Tracks amount of data being allocated

//-------------------------------------------------------------------------------------------------

// LOG_WARNING_LEVEL = filter for printing into the log file
// DEBUG_WARNING_LEVEL = filter for printing into the output window
// (Default = 3)

#define LOG_WARNING_LEVEL 3
#define DEBUG_WARNING_LEVEL 3

// Warning Level 0 - Use this to turn off tracking
// Warning Level 1 - Severe
// Warning Level 2 - Assert Recoverable
// Warning Level 3 - Default
// Warning Level 4 - Every Frame

//-------------------------------------------------------------------------------------------------

// MAX_LOG_HISTORY - Determines how many logs LoggerSystem will keep in /Data/Logs/...
// (Default = 10)

#define MAX_LOG_HISTORY 10

//-------------------------------------------------------------------------------------------------

// DEBUG_PROFILER - Profiler, tracks how long parts of the program takes

#define DEBUG_PROFILER 1

// 0 - Profiler disabled
// 1 - Profiler enabled

//-------------------------------------------------------------------------------------------------
#include "UnitaryTestMgr.h"
#include "Common.h"
#include "SharedDefines.h"
#include "Log.h"

#include "UnitaryTest_Misc.h"

static std::vector<UnitaryTest> tests =
{
    { DUMMY_TEST_ALWAYS_SUCCEED,  UNIT_TEST_CAT_MISC, UNIT_TEST_CAT_MISC_SUB_DUMMY, &MiscTests::Dummy_AlwaysSucceed },
    { DUMMY_TEST_ALWAYS_FAIL,     UNIT_TEST_CAT_MISC, UNIT_TEST_CAT_MISC_SUB_DUMMY, &MiscTests::Dummy_AlwaysFail    },
};

bool UnitaryTestMgr::Run(UnitaryTestId test)
{
    TC_LOG_INFO(UNIT_TEST_LOGGER, "***** Running test %u", test);

    bool success = tests[uint32(test)].Handler();

    if(success)
        TC_LOG_INFO(UNIT_TEST_LOGGER, "***** Test %u succeeded!", test);
    //failure infos is handled in tests

    return success;
}

bool UnitaryTestMgr::RunAllTests()
{
    uint32 successes = 0;
    uint32 total = 0;

    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Running ALL tests");

    for (auto itr : tests)
    {
        total++;
        bool success = Run(itr.id);
        if (success)
            successes++;
    }

    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Finished running ALL tests");
    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Total tests: %u", total);
    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Succeded tests: %u", successes);
    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Failed tests: %u", total-successes);

    return total == successes;
}

bool UnitaryTestMgr::RunCategoryTest(UnitaryTestCategory category, uint32 subCat)
{
    uint32 successes = 0;
    uint32 total = 0;

    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Running all tests for category %u", category);

    for (auto itr : tests)
    {
        if (itr.category != category)
            continue;

        if (subCat && itr.sub_category == subCat)
            continue;

        total++;
        bool success = Run(itr.id);
        if (success)
            successes++;
    }

    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Finished running all tests for category %u", category);
    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Total tests: %u", total);
    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Succeded tests: %u", successes);
    TC_LOG_INFO(UNIT_TEST_LOGGER, "********* Failed tests: %u", total - successes);

    return total == successes;
}

void UnitaryTestMgr::TestFailed(std::string comment)
{
    TC_LOG_ERROR(UNIT_TEST_LOGGER, "***** Test failed: %s", comment.c_str());
#if PLATFORM == PLATFORM_WINDOWS
    typedef USHORT(WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));

    if (func == NULL)
        return; // WOE 29.SEP.2010

                // Quote from Microsoft Documentation:
                // ## Windows Server 2003 and Windows XP:  
                // ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
    const int kMaxCallers = 62;

    void* callers[kMaxCallers];
    int count = (func)(0, kMaxCallers, callers, NULL);
    for (uint32 i = 0; i < count; i++)
        TC_LOG_ERROR(UNIT_TEST_LOGGER, "* %d called from %016I64LX", i, callers[i]);
#endif

#if PLATFORM == PLATFORM_UNIX
    void* arr[20];
    size_t size = backtrace(arr, 20);
    //print to stderr
    backtrace_symbols_fd(arr, size, STDERR_FILENO);

    //get backtrace as string array
    char** backtrace = backtrace_symbols(arr, size);

    for (int i = 0; i < size; i++)
        TC_LOG_ERROR(UNIT_TEST_LOGGER, "* %d called from %s", i, backtrace[i]);

    //delete array allocated by backtrace_symbols
    //delete backtrace;
#endif

}
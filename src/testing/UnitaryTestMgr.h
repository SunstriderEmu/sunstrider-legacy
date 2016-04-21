#ifndef TRINITY_UNITARYTESTMGR_H
#define TRINITY_UNITARYTESTMGR_H

#include "Common.h"
class UnitaryTestMgr;

enum UnitaryTestId
{
    DUMMY_TEST_ALWAYS_SUCCEED = 0,
    DUMMY_TEST_ALWAYS_FAIL,
};

enum UnitaryTestCategory
{
    UNIT_TEST_CAT_MISC,
    UNIT_TEST_CAT_SPELLS,
};

enum UnitaryTestCategory_Misc
{
    UNIT_TEST_CAT_MISC_SUB_DUMMY = 1,
};

enum UnitaryTestCategory_Spells
{
    UNIT_TEST_CAT_SPELLS_SUB_GENERAL = 1,
    UNIT_TEST_CAT_SPELLS_SUB_EFFECTS,
    UNIT_TEST_CAT_SPELLS_SUB_AURAS,
};

struct UnitaryTest
{
    UnitaryTestId id;
    UnitaryTestCategory category;
    uint32 sub_category;
    bool (*Handler)();
};

//enum TestImportance .. critical & co

#define UNIT_TEST_LOGGER "test.unit_test"

class UnitaryTestMgr
{
public:
       
    static bool Run(UnitaryTestId test);

    static bool RunAllTests();
    //subCat == 0 for all subcats
    static bool RunCategoryTest(UnitaryTestCategory category, uint32 subCat = 0);

    //print stacktrace to UNIT_TEST_LOGGER at error level
    static void TestFailed(std::string comment);
};

#define TEST(expression, comment) if(expression == false) { UnitaryTestMgr::TestFailed(comment); return false; }

#endif //TRINITY_UNITARYTESTMGR_H
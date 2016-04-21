#include "UnitaryTestMgr.h"
#include "UnitaryTest_Misc.h"

namespace MiscTests
{
	bool Dummy_AlwaysSucceed()
	{
        TEST(true, "This is always true");

		return true;
	}

    bool Dummy_AlwaysFail()
    {
        TEST(false, "This is always false");

        return true;
    }
}